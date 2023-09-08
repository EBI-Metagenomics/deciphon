#include "vit.h"
#include "array_size.h"
#include "array_size_field.h"
#include "compiler.h"
#include "imm/imm.h"
#include "p7.h"
#include "scan_thrd.h"
#include <stdlib.h>
#include <string.h>

#define PAST_SIZE 6

#define M_PAST_SIZE PAST_SIZE
#define I_PAST_SIZE PAST_SIZE
#define D_PAST_SIZE PAST_SIZE

#define lookback(i) (PAST_SIZE - 1 - (i))
#define emis_idx(nchars) ((nchars)-1)

#define CORE_OFFSET(k) (k * M_PAST_SIZE * I_PAST_SIZE * D_PAST_SIZE)
#define M_OFFSET(k) (CORE_OFFSET(k))
#define I_OFFSET(k) (CORE_OFFSET(k) + M_PAST_SIZE)
#define D_OFFSET(k) (CORE_OFFSET(k) + M_PAST_SIZE + I_PAST_SIZE)
#define DP_SIZE(core_size) CORE_OFFSET(core_size)

#define DP_M(dp, k, i) dp[M_OFFSET(k) + i]
#define DP_I(dp, k, i) dp[I_OFFSET(k) + i]
#define DP_D(dp, k, i) dp[D_OFFSET(k) + i]

static inline float reduce_max(int size, float const x[])
{
  float max = -INFINITY;

  for (int i = 0; i < size; i++)
    max = fmaxf(max, x[i]);

  return max;
}

dcp_pure_template int emission_index(struct imm_eseq const *eseq,
                                     int sequence_position, int emission_length)
{
  if (sequence_position < 0) return -1;
  return imm_eseq_get(eseq, sequence_position, emission_length, 1);
}

static inline float onto_R(float const *restrict dp_s,
                           float const *restrict dp_r, float const trans_sr,
                           float const trans_rr, float const *restrict emis)
{
  float const x[] = {
      dp_s[lookback(1)] + trans_sr + emis[emis_idx(1)],
      dp_s[lookback(2)] + trans_sr + emis[emis_idx(2)],
      dp_s[lookback(3)] + trans_sr + emis[emis_idx(3)],
      dp_s[lookback(4)] + trans_sr + emis[emis_idx(4)],
      dp_s[lookback(5)] + trans_sr + emis[emis_idx(5)],

      dp_r[lookback(1)] + trans_rr + emis[emis_idx(1)],
      dp_r[lookback(2)] + trans_rr + emis[emis_idx(2)],
      dp_r[lookback(3)] + trans_rr + emis[emis_idx(3)],
      dp_r[lookback(4)] + trans_rr + emis[emis_idx(4)],
      dp_r[lookback(5)] + trans_rr + emis[emis_idx(5)],
  };
  return reduce_max(array_size(x), x);
}

static inline float onto_N(float const *restrict dp_s,
                           float const *restrict dp_n, float const trans_sn,
                           float const trans_nn, float const *restrict emis)
{
  float const x[] = {
      dp_s[lookback(1)] + trans_sn + emis[emis_idx(1)],
      dp_s[lookback(2)] + trans_sn + emis[emis_idx(2)],
      dp_s[lookback(3)] + trans_sn + emis[emis_idx(3)],
      dp_s[lookback(4)] + trans_sn + emis[emis_idx(4)],
      dp_s[lookback(5)] + trans_sn + emis[emis_idx(5)],

      dp_n[lookback(1)] + trans_nn + emis[emis_idx(1)],
      dp_n[lookback(2)] + trans_nn + emis[emis_idx(2)],
      dp_n[lookback(3)] + trans_nn + emis[emis_idx(3)],
      dp_n[lookback(4)] + trans_nn + emis[emis_idx(4)],
      dp_n[lookback(5)] + trans_nn + emis[emis_idx(5)],
  };
  return reduce_max(array_size(x), x);
}

static inline float onto_B(float const *restrict dp_s,
                           float const *restrict dp_n,
                           float const *restrict dp_e,
                           float const *restrict dp_j, float const trans_sb,
                           float const trans_nb, float const trans_eb,
                           float const trans_jb, float const emis)
{
  float const x[] = {
      dp_s[lookback(0)] + trans_sb + emis,
      dp_n[lookback(0)] + trans_nb + emis,
      dp_e[lookback(0)] + trans_eb + emis,
      dp_j[lookback(0)] + trans_jb + emis,
  };
  return reduce_max(array_size(x), x);
}

static inline float onto_M1(float const *restrict dp_B, float const trans_BM,
                            float const *restrict emis_M)
{
  float const x[] = {
      dp_B[lookback(1)] + trans_BM + emis_M[emis_idx(1)],
      dp_B[lookback(2)] + trans_BM + emis_M[emis_idx(2)],
      dp_B[lookback(3)] + trans_BM + emis_M[emis_idx(3)],
      dp_B[lookback(4)] + trans_BM + emis_M[emis_idx(4)],
      dp_B[lookback(5)] + trans_BM + emis_M[emis_idx(5)],
  };
  return reduce_max(array_size(x), x);
}

static inline float onto_M(float const *restrict dp, float const *restrict dp_B,
                           struct dcp_trans const *restrict trans,
                           float const *restrict trans_BMk,
                           float const *restrict emis_M, int const k)
{
  float const x[] = {
      dp_B[lookback(1)] + trans_BMk[k + 1] + emis_M[emis_idx(1)],
      dp_B[lookback(2)] + trans_BMk[k + 1] + emis_M[emis_idx(2)],
      dp_B[lookback(3)] + trans_BMk[k + 1] + emis_M[emis_idx(3)],
      dp_B[lookback(4)] + trans_BMk[k + 1] + emis_M[emis_idx(4)],
      dp_B[lookback(5)] + trans_BMk[k + 1] + emis_M[emis_idx(5)],

      DP_M(dp, k, lookback(1)) + trans->MM + emis_M[emis_idx(1)],
      DP_M(dp, k, lookback(2)) + trans->MM + emis_M[emis_idx(2)],
      DP_M(dp, k, lookback(3)) + trans->MM + emis_M[emis_idx(3)],
      DP_M(dp, k, lookback(4)) + trans->MM + emis_M[emis_idx(4)],
      DP_M(dp, k, lookback(5)) + trans->MM + emis_M[emis_idx(5)],

      DP_I(dp, k, lookback(1)) + trans->IM + emis_M[emis_idx(1)],
      DP_I(dp, k, lookback(2)) + trans->IM + emis_M[emis_idx(2)],
      DP_I(dp, k, lookback(3)) + trans->IM + emis_M[emis_idx(3)],
      DP_I(dp, k, lookback(4)) + trans->IM + emis_M[emis_idx(4)],
      DP_I(dp, k, lookback(5)) + trans->IM + emis_M[emis_idx(5)],

      DP_D(dp, k, lookback(1)) + trans->DM + emis_M[emis_idx(1)],
      DP_D(dp, k, lookback(2)) + trans->DM + emis_M[emis_idx(2)],
      DP_D(dp, k, lookback(3)) + trans->DM + emis_M[emis_idx(3)],
      DP_D(dp, k, lookback(4)) + trans->DM + emis_M[emis_idx(4)],
      DP_D(dp, k, lookback(5)) + trans->DM + emis_M[emis_idx(5)],
  };
  return reduce_max(array_size(x), x);
}

static inline float onto_I(float const *restrict dp,
                           struct dcp_trans const *restrict trans,
                           float const *restrict emis_I, int const k)
{
  float const x[] = {
      DP_M(dp, k, lookback(1)) + trans->MI + emis_I[emis_idx(1)],
      DP_M(dp, k, lookback(2)) + trans->MI + emis_I[emis_idx(2)],
      DP_M(dp, k, lookback(3)) + trans->MI + emis_I[emis_idx(3)],
      DP_M(dp, k, lookback(4)) + trans->MI + emis_I[emis_idx(4)],
      DP_M(dp, k, lookback(5)) + trans->MI + emis_I[emis_idx(5)],

      DP_I(dp, k, lookback(1)) + trans->II + emis_I[emis_idx(1)],
      DP_I(dp, k, lookback(2)) + trans->II + emis_I[emis_idx(2)],
      DP_I(dp, k, lookback(3)) + trans->II + emis_I[emis_idx(3)],
      DP_I(dp, k, lookback(4)) + trans->II + emis_I[emis_idx(4)],
      DP_I(dp, k, lookback(5)) + trans->II + emis_I[emis_idx(5)],
  };
  return reduce_max(array_size(x), x);
}

static inline float onto_D(float const *restrict dp,
                           struct dcp_trans const *restrict trans,
                           float const emis_D, int const k)
{
  float const x[] = {
      DP_M(dp, k, lookback(0)) + trans->MD + emis_D,
      DP_D(dp, k, lookback(0)) + trans->DD + emis_D,
  };
  return reduce_max(array_size(x), x);
}

static inline float onto_E(float const *restrict dp, int const core_size,
                           float const trans_me, float const trans_de,
                           float const emis)
{
  float x = DP_M(dp, 0, lookback(0)) + trans_me + emis;
  for (int k = 1; k < core_size; ++k)
  {
    x = fmax(x, DP_M(dp, k, lookback(0)) + trans_me + emis);
    x = fmax(x, DP_D(dp, k, lookback(0)) + trans_de + emis);
  }
  return x;
}

static inline float onto_J(float const *restrict dp_e,
                           float const *restrict dp_j, float const trans_ej,
                           float const trans_jj, float const *restrict emis)
{
  float const x[] = {
      dp_e[lookback(1)] + trans_ej + emis[emis_idx(1)],
      dp_e[lookback(2)] + trans_ej + emis[emis_idx(2)],
      dp_e[lookback(3)] + trans_ej + emis[emis_idx(3)],
      dp_e[lookback(4)] + trans_ej + emis[emis_idx(4)],
      dp_e[lookback(5)] + trans_ej + emis[emis_idx(5)],

      dp_j[lookback(1)] + trans_jj + emis[emis_idx(1)],
      dp_j[lookback(2)] + trans_jj + emis[emis_idx(2)],
      dp_j[lookback(3)] + trans_jj + emis[emis_idx(3)],
      dp_j[lookback(4)] + trans_jj + emis[emis_idx(4)],
      dp_j[lookback(5)] + trans_jj + emis[emis_idx(5)],
  };
  return reduce_max(array_size(x), x);
}

static inline float onto_C(float const *restrict dp_e,
                           float const *restrict dp_c, float const trans_ec,
                           float const trans_cc, float const *restrict emis)
{
  float const x[] = {
      dp_e[lookback(1)] + trans_ec + emis[emis_idx(1)],
      dp_e[lookback(2)] + trans_ec + emis[emis_idx(2)],
      dp_e[lookback(3)] + trans_ec + emis[emis_idx(3)],
      dp_e[lookback(4)] + trans_ec + emis[emis_idx(4)],
      dp_e[lookback(5)] + trans_ec + emis[emis_idx(5)],

      dp_c[lookback(1)] + trans_cc + emis[emis_idx(1)],
      dp_c[lookback(2)] + trans_cc + emis[emis_idx(2)],
      dp_c[lookback(3)] + trans_cc + emis[emis_idx(3)],
      dp_c[lookback(4)] + trans_cc + emis[emis_idx(4)],
      dp_c[lookback(5)] + trans_cc + emis[emis_idx(5)],
  };
  return reduce_max(array_size(x), x);
}

static inline float onto_T(float const *restrict dp_e,
                           float const *restrict dp_c, float const trans_et,
                           float const trans_ct, float const emis)
{
  float const tmp[] = {dp_e[lookback(0)] + trans_et + emis,
                       dp_c[lookback(0)] + trans_ct + emis};
  return reduce_max(array_size(tmp), tmp);
}

struct extra_trans
{
  float const SB;
  float const SN;
  float const NN;
  float const NB;

  float const ET;
  float const EC;
  float const CC;
  float const CT;

  float const EB;
  float const EJ;
  float const JJ;
  float const JB;

  float const ME;
  float const DE;
};

static struct extra_trans extra_trans(struct dcp_xtrans xt)
{
  return (struct extra_trans){
      .SB = xt.NB,
      .SN = xt.NN,
      .NN = xt.NN,
      .NB = xt.NB,

      .ET = xt.EC + xt.CT,
      .EC = xt.EC + xt.CC,
      .CC = xt.CC,
      .CT = xt.CT,

      .EB = xt.EJ + xt.JB,
      .EJ = xt.EJ + xt.JJ,
      .JJ = xt.JJ,
      .JB = xt.JB,

      .ME = IMM_LPROB_ONE,
      .DE = IMM_LPROB_ONE,
  };
}

static float null_emis_tmp(float const *restrict null_emission, int pos)
{
  if (pos < 0) return IMM_LPROB_ZERO;
  return null_emission[pos];
}

static float bg_emis_tmp(float const *restrict bg_emission, int pos)
{
  if (pos < 0) return IMM_LPROB_ZERO;
  return bg_emission[pos];
}

static float match_emis_tmp(float const *restrict match_emission, int pos)
{
  if (pos < 0) return IMM_LPROB_ZERO;
  return match_emission[pos];
}

float dcp_vit_null(struct p7 *x, struct imm_eseq const *eseq)
{
  int seq_size = (int)imm_eseq_size(eseq);
  float const *restrict null_emission = x->null.emission;
  float SR = IMM_LPROB_ONE;
  float RR = x->null.RR;

#define NINF IMM_LPROB_ZERO
  float dp_S[PAST_SIZE] = {NINF, NINF, NINF, NINF, NINF, x->start_lprob};
  float dp_R[PAST_SIZE] = {NINF, NINF, NINF, NINF, NINF, NINF};
#undef NINF

  for (int r = 0; r < seq_size + 1; ++r)
  {
    float const null[] = {
        null_emis_tmp(null_emission, emission_index(eseq, r - 1, 1)),
        null_emis_tmp(null_emission, emission_index(eseq, r - 2, 2)),
        null_emis_tmp(null_emission, emission_index(eseq, r - 3, 3)),
        null_emis_tmp(null_emission, emission_index(eseq, r - 4, 4)),
        null_emis_tmp(null_emission, emission_index(eseq, r - 5, 5))};

    dp_R[lookback(0)] = onto_R(dp_S, dp_R, SR, RR, null);
    memmove(dp_S, dp_S + 1, sizeof(float) * (PAST_SIZE - 1));
    memmove(dp_R, dp_R + 1, sizeof(float) * (PAST_SIZE - 1));
    dp_S[PAST_SIZE - 1] = IMM_LPROB_ZERO;
    // dp_R[PAST_SIZE - 1] = IMM_LPROB_ZERO;
  }
  return dp_R[5];
}

float dcp_vit(struct p7 *x, struct imm_eseq const *eseq)
{
  int core_size = x->core_size;
  int seq_size = (int)imm_eseq_size(eseq);

#define NINF IMM_LPROB_ZERO
  float *restrict dp = malloc(sizeof(*dp) * DP_SIZE(core_size));
  for (int i = 0; i < DP_SIZE(core_size); ++i)
    dp[i] = NINF;
  float dp_S[PAST_SIZE] = {NINF, NINF, NINF, NINF, NINF, x->start_lprob};
  float dp_N[PAST_SIZE] = {NINF, NINF, NINF, NINF, NINF, NINF};
  float dp_B[PAST_SIZE] = {NINF, NINF, NINF, NINF, NINF, NINF};
  float dp_J[PAST_SIZE] = {NINF, NINF, NINF, NINF, NINF, NINF};
  float dp_E[PAST_SIZE] = {NINF, NINF, NINF, NINF, NINF, NINF};
  float dp_C[PAST_SIZE] = {NINF, NINF, NINF, NINF, NINF, NINF};
  float dp_T[PAST_SIZE] = {NINF, NINF, NINF, NINF, NINF, NINF};
#undef NINF

  float const *restrict trans_BM = x->BMk;
  struct extra_trans const xtrans = extra_trans(x->xtrans);

  float const *restrict background_emission = x->bg.emission;
  float const mute_emission = IMM_LPROB_ONE;
  float const emis_B = mute_emission;
  float const emis_D = mute_emission;
  float const emis_E = mute_emission;
  float const emis_T = mute_emission;

  float const *restrict null_emission = x->null.emission;

  for (int r = 0; r < seq_size + 1; ++r)
  {
    int emis_idx[5];
    for (int i = 1; i <= 5; ++i)
      emis_idx[i - 1] = emission_index(eseq, r - i, i);

    float const null[] = {null_emis_tmp(null_emission, emis_idx[0]),
                          null_emis_tmp(null_emission, emis_idx[1]),
                          null_emis_tmp(null_emission, emis_idx[2]),
                          null_emis_tmp(null_emission, emis_idx[3]),
                          null_emis_tmp(null_emission, emis_idx[4])};

    float const bg[] = {bg_emis_tmp(background_emission, emis_idx[0]),
                        bg_emis_tmp(background_emission, emis_idx[1]),
                        bg_emis_tmp(background_emission, emis_idx[2]),
                        bg_emis_tmp(background_emission, emis_idx[3]),
                        bg_emis_tmp(background_emission, emis_idx[4])};

    float const *restrict emis_I = bg;
    float const *restrict emis_N = null;
    float const *restrict emis_J = null;
    float const *restrict emis_C = null;

    dp_E[lookback(0)] = onto_E(dp, core_size, xtrans.ME, xtrans.DE, emis_E);
    dp_N[lookback(0)] = onto_N(dp_S, dp_N, xtrans.SN, xtrans.NN, emis_N);
    dp_B[lookback(0)] = onto_B(dp_S, dp_N, dp_E, dp_J, xtrans.SB, xtrans.NB,
                               xtrans.EB, xtrans.JB, emis_B);

    {
      float const *match_emission = x->nodes[0].emission;
      float const emis_M[5] = {match_emis_tmp(match_emission, emis_idx[0]),
                               match_emis_tmp(match_emission, emis_idx[1]),
                               match_emis_tmp(match_emission, emis_idx[2]),
                               match_emis_tmp(match_emission, emis_idx[3]),
                               match_emis_tmp(match_emission, emis_idx[4])};
      DP_M(dp, 0, lookback(0)) = onto_M1(dp_B, trans_BM[0], emis_M);
    }

    for (int k = 0; k + 1 < core_size; ++k)
    {
      int const k0 = k;
      int const k1 = k + 1;
      struct dcp_trans const *restrict trans = &x->nodes[k0].trans;
      float const *match_emission = x->nodes[k1].emission;

      float const emis_M[5] = {match_emis_tmp(match_emission, emis_idx[0]),
                               match_emis_tmp(match_emission, emis_idx[1]),
                               match_emis_tmp(match_emission, emis_idx[2]),
                               match_emis_tmp(match_emission, emis_idx[3]),
                               match_emis_tmp(match_emission, emis_idx[4])};

      DP_I(dp, k0, lookback(0)) = onto_I(dp, trans, emis_I, k0);
      DP_M(dp, k1, lookback(0)) = onto_M(dp, dp_B, trans, trans_BM, emis_M, k0);
      DP_D(dp, k1, lookback(0)) = onto_D(dp, trans, emis_D, k0);
    }

    dp_E[lookback(0)] = onto_E(dp, core_size, xtrans.ME, xtrans.DE, emis_E);
    dp_J[lookback(0)] = onto_J(dp_E, dp_J, xtrans.EJ, xtrans.JJ, emis_J);
    dp_C[lookback(0)] = onto_C(dp_E, dp_C, xtrans.EC, xtrans.CC, emis_C);
    dp_T[lookback(0)] = onto_T(dp_E, dp_C, xtrans.ET, xtrans.CT, emis_T);

    memmove(dp_S, dp_S + 1, sizeof(float) * (PAST_SIZE - 1));
    dp_S[PAST_SIZE - 1] = IMM_LPROB_ZERO;
    memmove(dp_N, dp_N + 1, sizeof(float) * (PAST_SIZE - 1));
    memmove(dp_B, dp_B + 1, sizeof(float) * (PAST_SIZE - 1));
    memmove(dp_J, dp_J + 1, sizeof(float) * (PAST_SIZE - 1));
    memmove(dp_E, dp_E + 1, sizeof(float) * (PAST_SIZE - 1));
    memmove(dp_C, dp_C + 1, sizeof(float) * (PAST_SIZE - 1));
    memmove(dp_T, dp_T + 1, sizeof(float) * (PAST_SIZE - 1));
    for (int i = 0; i < core_size; ++i)
    {
      size_t count = sizeof(float) * (PAST_SIZE - 1);
      memmove(&DP_I(dp, i, lookback(5)), &DP_I(dp, i, lookback(4)), count);
      memmove(&DP_M(dp, i, lookback(5)), &DP_M(dp, i, lookback(4)), count);
      memmove(&DP_D(dp, i, lookback(5)), &DP_D(dp, i, lookback(4)), count);
    }
  }

  float score = dp_T[5];
  free(dp);
  return score;
}

void dcp_vit_dump(struct p7 *x, FILE *restrict fp)
{
  int core_size = x->core_size;

  float const mute_emission = IMM_LPROB_ONE;
  float const emis_B = mute_emission;
  float const emis_D = mute_emission;
  float const emis_E = mute_emission;
  float const emis_T = mute_emission;

  char const *f32f = imm_fmt_get_f32();

  fprintf(fp, "B: ");
  fprintf(fp, f32f, emis_B);
  fputc('\n', fp);

  fprintf(fp, "D: ");
  fprintf(fp, f32f, emis_D);
  fputc('\n', fp);

  fprintf(fp, "E: ");
  fprintf(fp, f32f, emis_E);
  fputc('\n', fp);

  fprintf(fp, "T: ");
  fprintf(fp, f32f, emis_T);
  fputc('\n', fp);

  float const *restrict null_emission = x->null.emission;
  float const *restrict background_emission = x->bg.emission;
  float const *restrict emis_I = background_emission;
  float const *restrict emis_N = null_emission;
  float const *restrict emis_J = null_emission;
  float const *restrict emis_C = null_emission;
  size_t bg_size = array_size_field(struct p7_background, emission);

  fprintf(fp, "I*: ");
  imm_dump_array_f32(bg_size, emis_I, fp);
  fputc('\n', fp);

  fprintf(fp, "N: [");
  imm_dump_array_f32(bg_size, emis_N, fp);
  fprintf(fp, "]\n");

  fprintf(fp, "J: [");
  imm_dump_array_f32(bg_size, emis_J, fp);
  fprintf(fp, "]\n");

  fprintf(fp, "C: [");
  imm_dump_array_f32(bg_size, emis_C, fp);
  fprintf(fp, "]\n");

  for (int k = 0; k < core_size; ++k)
  {
    float const *match_emission = x->nodes[k].emission;

    fprintf(fp, "M%d: ", k + 1);
    size_t n = array_size_field(struct p7_node, emission);
    imm_dump_array_f32(n, match_emission, fp);
    fputc('\n', fp);
  }
}

void dcp_vit_dump_dot(struct p7 *x, FILE *restrict fp)
{
  char const *f32f = imm_fmt_get_f32();

  struct extra_trans const xtrans = extra_trans(x->xtrans);

  fprintf(fp, "S -> B [label=");
  fprintf(fp, f32f, xtrans.SB);
  fprintf(fp, "];\n");

  fprintf(fp, "S -> N [label=");
  fprintf(fp, f32f, xtrans.SN);
  fprintf(fp, "];\n");

  fprintf(fp, "N -> N [label=");
  fprintf(fp, f32f, xtrans.NN);
  fprintf(fp, "];\n");

  fprintf(fp, "N -> B [label=");
  fprintf(fp, f32f, xtrans.NB);
  fprintf(fp, "];\n");

  fprintf(fp, "E -> T [label=");
  fprintf(fp, f32f, xtrans.ET);
  fprintf(fp, "];\n");

  fprintf(fp, "E -> C [label=");
  fprintf(fp, f32f, xtrans.EC);
  fprintf(fp, "];\n");

  fprintf(fp, "C -> C [label=");
  fprintf(fp, f32f, xtrans.CC);
  fprintf(fp, "];\n");

  fprintf(fp, "C -> T [label=");
  fprintf(fp, f32f, xtrans.CT);
  fprintf(fp, "];\n");

  fprintf(fp, "E -> B [label=");
  fprintf(fp, f32f, xtrans.EB);
  fprintf(fp, "];\n");

  fprintf(fp, "E -> J [label=");
  fprintf(fp, f32f, xtrans.EJ);
  fprintf(fp, "];\n");

  fprintf(fp, "J -> J [label=");
  fprintf(fp, f32f, xtrans.JJ);
  fprintf(fp, "];\n");

  fprintf(fp, "J -> B [label=");
  fprintf(fp, f32f, xtrans.JB);
  fprintf(fp, "];\n");

  int core_size = x->core_size;
  for (int k = 0; k + 1 < core_size; ++k)
  {
    struct dcp_trans const *restrict trans = &x->nodes[k].trans;
    int i0 = k + 1;
    int i1 = k + 2;
    fprintf(fp, "D%d -> D%d [label=", i0, i1);
    fprintf(fp, f32f, trans->DD);
    fprintf(fp, "];\n");

    fprintf(fp, "D%d -> M%d [label=", i0, i1);
    fprintf(fp, f32f, trans->DM);
    fprintf(fp, "];\n");

    fprintf(fp, "I%d -> I%d [label=", i0, i0);
    fprintf(fp, f32f, trans->II);
    fprintf(fp, "];\n");

    fprintf(fp, "I%d -> M%d [label=", i0, i1);
    fprintf(fp, f32f, trans->IM);
    fprintf(fp, "];\n");

    fprintf(fp, "M%d -> I%d [label=", i0, i0);
    fprintf(fp, f32f, trans->MI);
    fprintf(fp, "];\n");

    fprintf(fp, "M%d -> M%d [label=", i0, i1);
    fprintf(fp, f32f, trans->MM);
    fprintf(fp, "];\n");

    fprintf(fp, "M%d -> D%d [label=", i0, i1);
    fprintf(fp, f32f, trans->MD);
    fprintf(fp, "];\n");

    fprintf(fp, "M%d -> E [label=", i0);
    fprintf(fp, f32f, xtrans.ME);
    fprintf(fp, "];\n");

    fprintf(fp, "D%d -> E [label=", i0);
    fprintf(fp, f32f, xtrans.DE);
    fprintf(fp, "];\n");
  }

  fprintf(fp, "M%d -> E [label=", core_size);
  fprintf(fp, f32f, xtrans.ME);
  fprintf(fp, "];\n");

  fprintf(fp, "D%d -> E [label=", core_size);
  fprintf(fp, f32f, xtrans.DE);
  fprintf(fp, "];\n");

  float const *restrict trans_BM = x->BMk;
  for (int k = 0; k < core_size; ++k)
  {
    fprintf(fp, "B -> M%d [label=", k + 1);
    fprintf(fp, f32f, trans_BM[k]);
    fprintf(fp, "];\n");
  }
}
