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

#define lukbak(i) ((i))
#define nchars(n) ((n)-1)

#define CORE_OFFSET(k) ((k)*M_PAST_SIZE * I_PAST_SIZE * D_PAST_SIZE)
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

DCP_PURE int emission_index(struct imm_eseq const *eseq, int sequence_position,
                            int emission_length)
{
  if (sequence_position < 0) return -1;
  return imm_eseq_get(eseq, sequence_position, emission_length, 1);
}

static inline float onto_R(float const *restrict dp_s,
                           float const *restrict dp_r, float const trans_sr,
                           float const trans_rr, float const *restrict emis)
{
  float const x[] = {
      dp_s[lukbak(1)] + trans_sr + emis[nchars(1)],
      dp_s[lukbak(2)] + trans_sr + emis[nchars(2)],
      dp_s[lukbak(3)] + trans_sr + emis[nchars(3)],
      dp_s[lukbak(4)] + trans_sr + emis[nchars(4)],
      dp_s[lukbak(5)] + trans_sr + emis[nchars(5)],

      dp_r[lukbak(1)] + trans_rr + emis[nchars(1)],
      dp_r[lukbak(2)] + trans_rr + emis[nchars(2)],
      dp_r[lukbak(3)] + trans_rr + emis[nchars(3)],
      dp_r[lukbak(4)] + trans_rr + emis[nchars(4)],
      dp_r[lukbak(5)] + trans_rr + emis[nchars(5)],
  };
  return reduce_max(array_size(x), x);
}

static inline float onto_N(float const *restrict dp_s,
                           float const *restrict dp_n, float const trans_sn,
                           float const trans_nn, float const *restrict emis)
{
  float const x[] = {
      dp_s[lukbak(1)] + trans_sn + emis[nchars(1)],
      dp_s[lukbak(2)] + trans_sn + emis[nchars(2)],
      dp_s[lukbak(3)] + trans_sn + emis[nchars(3)],
      dp_s[lukbak(4)] + trans_sn + emis[nchars(4)],
      dp_s[lukbak(5)] + trans_sn + emis[nchars(5)],

      dp_n[lukbak(1)] + trans_nn + emis[nchars(1)],
      dp_n[lukbak(2)] + trans_nn + emis[nchars(2)],
      dp_n[lukbak(3)] + trans_nn + emis[nchars(3)],
      dp_n[lukbak(4)] + trans_nn + emis[nchars(4)],
      dp_n[lukbak(5)] + trans_nn + emis[nchars(5)],
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
      dp_s[lukbak(0)] + trans_sb + emis,
      dp_n[lukbak(0)] + trans_nb + emis,
      dp_e[lukbak(0)] + trans_eb + emis,
      dp_j[lukbak(0)] + trans_jb + emis,
  };
  return reduce_max(array_size(x), x);
}

static inline float onto_M1(float const *restrict dp_B, float const trans_BM,
                            float const *restrict M)
{
  float const x[] = {
      dp_B[lukbak(1)] + trans_BM + M[nchars(1)],
      dp_B[lukbak(2)] + trans_BM + M[nchars(2)],
      dp_B[lukbak(3)] + trans_BM + M[nchars(3)],
      dp_B[lukbak(4)] + trans_BM + M[nchars(4)],
      dp_B[lukbak(5)] + trans_BM + M[nchars(5)],
  };
  return reduce_max(array_size(x), x);
}

static inline float onto_M(float const DPM[restrict], float const DPI[restrict],
                           float const DPD[restrict], float const *restrict B,
                           float const MM, float const IM, float const DM,
                           float const BM, float const *restrict M)
{
  // clang-format off
  float const x[] = {
      B[lukbak(1)] + BM + M[nchars(1)],
      B[lukbak(2)] + BM + M[nchars(2)],
      B[lukbak(3)] + BM + M[nchars(3)],
      B[lukbak(4)] + BM + M[nchars(4)],
      B[lukbak(5)] + BM + M[nchars(5)],

      DPM[lukbak(1)] + MM + M[nchars(1)],
      DPM[lukbak(2)] + MM + M[nchars(2)],
      DPM[lukbak(3)] + MM + M[nchars(3)],
      DPM[lukbak(4)] + MM + M[nchars(4)],
      DPM[lukbak(5)] + MM + M[nchars(5)],

      DPI[lukbak(1)] + IM + M[nchars(1)],
      DPI[lukbak(2)] + IM + M[nchars(2)],
      DPI[lukbak(3)] + IM + M[nchars(3)],
      DPI[lukbak(4)] + IM + M[nchars(4)],
      DPI[lukbak(5)] + IM + M[nchars(5)],

      DPD[lukbak(1)] + DM + M[nchars(1)],
      DPD[lukbak(2)] + DM + M[nchars(2)],
      DPD[lukbak(3)] + DM + M[nchars(3)],
      DPD[lukbak(4)] + DM + M[nchars(4)],
      DPD[lukbak(5)] + DM + M[nchars(5)],
  };
  // clang-format on
  return reduce_max(array_size(x), x);
}

static inline float onto_I(float const DPM[restrict], float const DPI[restrict],
                           float const MI, float const II,
                           float const *restrict I)
{
  // clang-format off
  float const x[] = {
      DPM[lukbak(1)] + MI + I[nchars(1)],
      DPM[lukbak(2)] + MI + I[nchars(2)],
      DPM[lukbak(3)] + MI + I[nchars(3)],
      DPM[lukbak(4)] + MI + I[nchars(4)],
      DPM[lukbak(5)] + MI + I[nchars(5)],

      DPI[lukbak(1)] + II + I[nchars(1)],
      DPI[lukbak(2)] + II + I[nchars(2)],
      DPI[lukbak(3)] + II + I[nchars(3)],
      DPI[lukbak(4)] + II + I[nchars(4)],
      DPI[lukbak(5)] + II + I[nchars(5)],
  };
  // clang-format on
  return reduce_max(array_size(x), x);
}

static inline float onto_D(float const DPM[restrict], float const DPD[restrict],
                           float const MD, float const DD, float const D)
{
  // clang-format off
  float const x[] = {
      DPM[lukbak(0)] + MD + D,
      DPD[lukbak(0)] + DD + D,
  };
  // clang-format on
  return reduce_max(array_size(x), x);
}

static inline float onto_E(float const *restrict dp, int const core_size,
                           float const trans_me, float const trans_de,
                           float const emis)
{
  // Using lukbak(1) because I'm already in the future (as per make_future)
  // It would be lukbak(0) if I hadn't call make_future on DP_M and DP_D
  float x = DP_M(dp, 0, lukbak(1)) + trans_me + emis;
  for (int k = 1; k < core_size; ++k)
  {
    x = fmax(x, DP_M(dp, k, lukbak(1)) + trans_me + emis);
    x = fmax(x, DP_D(dp, k, lukbak(1)) + trans_de + emis);
  }
  return x;
}

static inline float onto_J(float const *restrict dp_e,
                           float const *restrict dp_j, float const trans_ej,
                           float const trans_jj, float const *restrict emis)
{
  float const x[] = {
      dp_e[lukbak(1)] + trans_ej + emis[nchars(1)],
      dp_e[lukbak(2)] + trans_ej + emis[nchars(2)],
      dp_e[lukbak(3)] + trans_ej + emis[nchars(3)],
      dp_e[lukbak(4)] + trans_ej + emis[nchars(4)],
      dp_e[lukbak(5)] + trans_ej + emis[nchars(5)],

      dp_j[lukbak(1)] + trans_jj + emis[nchars(1)],
      dp_j[lukbak(2)] + trans_jj + emis[nchars(2)],
      dp_j[lukbak(3)] + trans_jj + emis[nchars(3)],
      dp_j[lukbak(4)] + trans_jj + emis[nchars(4)],
      dp_j[lukbak(5)] + trans_jj + emis[nchars(5)],
  };
  return reduce_max(array_size(x), x);
}

static inline float onto_C(float const *restrict dp_e,
                           float const *restrict dp_c, float const trans_ec,
                           float const trans_cc, float const *restrict emis)
{
  float const x[] = {
      dp_e[lukbak(1)] + trans_ec + emis[nchars(1)],
      dp_e[lukbak(2)] + trans_ec + emis[nchars(2)],
      dp_e[lukbak(3)] + trans_ec + emis[nchars(3)],
      dp_e[lukbak(4)] + trans_ec + emis[nchars(4)],
      dp_e[lukbak(5)] + trans_ec + emis[nchars(5)],

      dp_c[lukbak(1)] + trans_cc + emis[nchars(1)],
      dp_c[lukbak(2)] + trans_cc + emis[nchars(2)],
      dp_c[lukbak(3)] + trans_cc + emis[nchars(3)],
      dp_c[lukbak(4)] + trans_cc + emis[nchars(4)],
      dp_c[lukbak(5)] + trans_cc + emis[nchars(5)],
  };
  return reduce_max(array_size(x), x);
}

static inline float onto_T(float const *restrict dp_e,
                           float const *restrict dp_c, float const trans_et,
                           float const trans_ct, float const emis)
{
  float const tmp[] = {dp_e[lukbak(0)] + trans_et + emis,
                       dp_c[lukbak(0)] + trans_ct + emis};
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

DCP_PURE float safe_get(float const x[restrict], int pos)
{
  return pos >= 0 ? x[pos] : IMM_LPROB_ZERO;
}

float dcp_vit_null(struct p7 *x, struct imm_eseq const *eseq)
{
  int seq_size = (int)imm_eseq_size(eseq);
  float const *restrict null_emission = x->null.emission;
  float SR = IMM_LPROB_ONE;
  float RR = x->null.RR;

#define NINF IMM_LPROB_ZERO
  float dp_S[PAST_SIZE] = {NINF, NINF, NINF, NINF, NINF, NINF};
  float dp_R[PAST_SIZE] = {NINF, NINF, NINF, NINF, NINF, NINF};
#undef NINF
  dp_S[lukbak(0)] = 0;

  for (int r = 0; r < seq_size + 1; ++r)
  {
    float const null[] = {
        safe_get(null_emission, emission_index(eseq, r - 1, 1)),
        safe_get(null_emission, emission_index(eseq, r - 2, 2)),
        safe_get(null_emission, emission_index(eseq, r - 3, 3)),
        safe_get(null_emission, emission_index(eseq, r - 4, 4)),
        safe_get(null_emission, emission_index(eseq, r - 5, 5))};

    dp_R[lukbak(0)] = onto_R(dp_S, dp_R, SR, RR, null);
    size_t sz = sizeof(float) * (PAST_SIZE - 1);
    memmove(dp_S + lukbak(1), dp_S + lukbak(0), sz);
    memmove(dp_R + lukbak(1), dp_R + lukbak(0), sz);
    dp_S[lukbak(0)] = IMM_LPROB_ZERO;
    // dp_R[PAST_SIZE - 1] = IMM_LPROB_ZERO;
  }
  return dp_R[lukbak(0)];
}

static inline void make_future(float x[])
{
  memmove(&x[lukbak(1)], &x[lukbak(0)], sizeof(float) * (PAST_SIZE - 1));
}

float dcp_vit(struct p7 *x, struct imm_eseq const *eseq)
{
  int core_size = x->core_size;
  int seq_size = (int)imm_eseq_size(eseq);

#define NINF IMM_LPROB_ZERO
  float *restrict dp = malloc(sizeof(*dp) * DP_SIZE(core_size));
  for (int i = 0; i < DP_SIZE(core_size); ++i)
    dp[i] = NINF;
  float S[PAST_SIZE] = {NINF, NINF, NINF, NINF, NINF, NINF};
  float N[PAST_SIZE] = {NINF, NINF, NINF, NINF, NINF, NINF};
  float B[PAST_SIZE] = {NINF, NINF, NINF, NINF, NINF, NINF};
  float J[PAST_SIZE] = {NINF, NINF, NINF, NINF, NINF, NINF};
  float E[PAST_SIZE] = {NINF, NINF, NINF, NINF, NINF, NINF};
  float C[PAST_SIZE] = {NINF, NINF, NINF, NINF, NINF, NINF};
  float T[PAST_SIZE] = {NINF, NINF, NINF, NINF, NINF, NINF};
#undef NINF
  S[lukbak(0)] = 0;

  float const *restrict BM = x->BMk;
  struct extra_trans const xt = extra_trans(x->xtrans);

  for (int r = 0; r < seq_size + 1; ++r)
  {
    int ix[5];
    for (int i = 1; i <= 5; ++i)
      ix[i - 1] = emission_index(eseq, r - i, i);

    float const null[] = {safe_get(x->null.emission, ix[nchars(1)]),
                          safe_get(x->null.emission, ix[nchars(2)]),
                          safe_get(x->null.emission, ix[nchars(3)]),
                          safe_get(x->null.emission, ix[nchars(4)]),
                          safe_get(x->null.emission, ix[nchars(5)])};

    float const bg[] = {safe_get(x->bg.emission, ix[nchars(1)]),
                        safe_get(x->bg.emission, ix[nchars(2)]),
                        safe_get(x->bg.emission, ix[nchars(3)]),
                        safe_get(x->bg.emission, ix[nchars(4)]),
                        safe_get(x->bg.emission, ix[nchars(5)])};

    E[lukbak(0)] = onto_E(dp, core_size, xt.ME, xt.DE, 0);
    N[lukbak(0)] = onto_N(S, N, xt.SN, xt.NN, null);
    B[lukbak(0)] = onto_B(S, N, E, J, xt.SB, xt.NB, xt.EB, xt.JB, 0);
    make_future(S);
    make_future(N);

    float const M[] = {safe_get(x->nodes[0].emission, ix[nchars(1)]),
                       safe_get(x->nodes[0].emission, ix[nchars(2)]),
                       safe_get(x->nodes[0].emission, ix[nchars(3)]),
                       safe_get(x->nodes[0].emission, ix[nchars(4)]),
                       safe_get(x->nodes[0].emission, ix[nchars(5)])};
    float *DPM = &DP_M(dp, 0, lukbak(0));
    float *DPI = &DP_I(dp, 0, lukbak(0));
    float *DPD = &DP_D(dp, 0, lukbak(0));
    DPM[lukbak(0)] = onto_M1(B, BM[0], M);

    for (int k = 0; k + 1 < core_size; ++k)
    {
      int const k0 = k;
      int const k1 = k + 1;
      struct dcp_trans const *restrict t = &x->nodes[k0].trans;

      float const M[] = {safe_get(x->nodes[k1].emission, ix[nchars(1)]),
                         safe_get(x->nodes[k1].emission, ix[nchars(2)]),
                         safe_get(x->nodes[k1].emission, ix[nchars(3)]),
                         safe_get(x->nodes[k1].emission, ix[nchars(4)]),
                         safe_get(x->nodes[k1].emission, ix[nchars(5)])};

      DPI[lukbak(0)] = onto_I(DPM, DPI, t->MI, t->II, bg);
      float tmpM = onto_M(DPM, DPI, DPD, B, t->MM, t->IM, t->DM, BM[k1], M);
      float tmpD = onto_D(DPM, DPD, t->MD, t->DD, 0);
      make_future(DPM);
      make_future(DPI);
      make_future(DPD);
      DPM = &DP_M(dp, k1, lukbak(0));
      DPM[lukbak(0)] = tmpM;
      DPI = &DP_I(dp, k1, lukbak(0));
      DPD = &DP_D(dp, k1, lukbak(0));
      DPD[lukbak(0)] = tmpD;
    }
    make_future(DPM);
    make_future(DPI);
    make_future(DPD);
    make_future(B);

    E[lukbak(0)] = onto_E(dp, core_size, xt.ME, xt.DE, 0);
    J[lukbak(0)] = onto_J(E, J, xt.EJ, xt.JJ, null);
    make_future(J);
    C[lukbak(0)] = onto_C(E, C, xt.EC, xt.CC, null);
    T[lukbak(0)] = onto_T(E, C, xt.ET, xt.CT, 0);
    make_future(E);
    make_future(C);
    make_future(T);
    S[lukbak(0)] = IMM_LPROB_ZERO;
  }

  float score = T[lukbak(0)];
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
