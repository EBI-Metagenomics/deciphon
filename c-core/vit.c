#include "vit.h"
#include "array_size.h"
#include "array_size_field.h"
#include "compiler.h"
#include "imm/imm.h"
#include "p7.h"
#include "reduce.h"
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

#define LOCALITY 1

#define emission_index(a, b, c, safe)                                          \
  ((!safe && (b) < 0) ? -1 : (int)imm_eseq_get((a), (b), (c), 1))

#define safe_get(x, i, safe_range)                                             \
  (safe_range ? (x)[(i)] : (i) >= 0 ? (x)[(i)] : IMM_LPROB_ZERO)

static inline float onto_R(float const S[restrict], float const R[restrict],
                           float const RR, float const emis[restrict])
{
  // clang-format off
  float const x[] = {
      S[lukbak(1)] + 0 + emis[nchars(1)],
      S[lukbak(2)] + 0 + emis[nchars(2)],
      S[lukbak(3)] + 0 + emis[nchars(3)],
      S[lukbak(4)] + 0 + emis[nchars(4)],
      S[lukbak(5)] + 0 + emis[nchars(5)],

      R[lukbak(1)] + RR + emis[nchars(1)],
      R[lukbak(2)] + RR + emis[nchars(2)],
      R[lukbak(3)] + RR + emis[nchars(3)],
      R[lukbak(4)] + RR + emis[nchars(4)],
      R[lukbak(5)] + RR + emis[nchars(5)],
  };
  // clang-format on
  return reduce_fmax(array_size(x), x);
}

static inline float onto_N(float const S[restrict], float const N[restrict],
                           float const SN, float const NN,
                           float const emis[restrict])
{
  // clang-format off
  float const x[] = {
      S[lukbak(1)] + SN + emis[nchars(1)],
      S[lukbak(2)] + SN + emis[nchars(2)],
      S[lukbak(3)] + SN + emis[nchars(3)],
      S[lukbak(4)] + SN + emis[nchars(4)],
      S[lukbak(5)] + SN + emis[nchars(5)],

      N[lukbak(1)] + NN + emis[nchars(1)],
      N[lukbak(2)] + NN + emis[nchars(2)],
      N[lukbak(3)] + NN + emis[nchars(3)],
      N[lukbak(4)] + NN + emis[nchars(4)],
      N[lukbak(5)] + NN + emis[nchars(5)],
  };
  // clang-format on
  return reduce_fmax(array_size(x), x);
}

static inline float onto_B(float const S[restrict], float const N[restrict],
                           float const E[restrict], float const J[restrict],
                           float const SB, float const NB, float const EB,
                           float const JB, float const emis)
{
  // clang-format off
  float const x[] = {
      S[lukbak(0)] + SB + emis,
      N[lukbak(0)] + NB + emis,
      E[lukbak(0)] + EB + emis,
      J[lukbak(0)] + JB + emis,
  };
  // clang-format on
  return reduce_fmax(array_size(x), x);
}

static inline float onto_M1(float const B[restrict], float const BM,
                            float const emis[restrict])
{
  // clang-format off
  float const x[] = {
      B[lukbak(1)] + BM + emis[nchars(1)],
      B[lukbak(2)] + BM + emis[nchars(2)],
      B[lukbak(3)] + BM + emis[nchars(3)],
      B[lukbak(4)] + BM + emis[nchars(4)],
      B[lukbak(5)] + BM + emis[nchars(5)],
  };
  // clang-format on
  return reduce_fmax(array_size(x), x);
}

DCP_PURE float onto_M(float const M[restrict], float const I[restrict],
                      float const D[restrict], float const B[restrict],
                      float const MM, float const IM, float const DM,
                      float const BM, float const emis[restrict])
{
  // clang-format off
  float const x[] = {
      B[lukbak(1)] + BM + emis[nchars(1)],
      B[lukbak(2)] + BM + emis[nchars(2)],
      B[lukbak(3)] + BM + emis[nchars(3)],
      B[lukbak(4)] + BM + emis[nchars(4)],
      B[lukbak(5)] + BM + emis[nchars(5)],

      M[lukbak(1)] + MM + emis[nchars(1)],
      M[lukbak(2)] + MM + emis[nchars(2)],
      M[lukbak(3)] + MM + emis[nchars(3)],
      M[lukbak(4)] + MM + emis[nchars(4)],
      M[lukbak(5)] + MM + emis[nchars(5)],

      I[lukbak(1)] + IM + emis[nchars(1)],
      I[lukbak(2)] + IM + emis[nchars(2)],
      I[lukbak(3)] + IM + emis[nchars(3)],
      I[lukbak(4)] + IM + emis[nchars(4)],
      I[lukbak(5)] + IM + emis[nchars(5)],

      D[lukbak(1)] + DM + emis[nchars(1)],
      D[lukbak(2)] + DM + emis[nchars(2)],
      D[lukbak(3)] + DM + emis[nchars(3)],
      D[lukbak(4)] + DM + emis[nchars(4)],
      D[lukbak(5)] + DM + emis[nchars(5)],
  };
  // clang-format on
  return reduce_fmax(array_size(x), x);
}

DCP_PURE float onto_I(float const DPMI[restrict], float const MI,
                      float const II, float const emis[restrict])
{
  // clang-format off
  float const x[] = {
      DPMI[lukbak(1)] + MI + emis[nchars(1)],
      DPMI[lukbak(2)] + MI + emis[nchars(2)],
      DPMI[lukbak(3)] + MI + emis[nchars(3)],
      DPMI[lukbak(4)] + MI + emis[nchars(4)],
      DPMI[lukbak(5)] + MI + emis[nchars(5)],
      DPMI[lukbak(5)+lukbak(1)] + II + emis[nchars(1)],
      DPMI[lukbak(5)+lukbak(2)] + II + emis[nchars(2)],
      DPMI[lukbak(5)+lukbak(3)] + II + emis[nchars(3)],
      DPMI[lukbak(5)+lukbak(4)] + II + emis[nchars(4)],
      DPMI[lukbak(5)+lukbak(5)] + II + emis[nchars(5)],
  };
  // clang-format on
  return reduce_fmax(array_size(x), x);
}

DCP_PURE float onto_D(float const DPM[restrict], float const DPD[restrict],
                      float const MD, float const DD, float const D)
{
  // clang-format off
  float const x[] = {
      DPM[lukbak(0)] + MD + D,
      DPD[lukbak(0)] + DD + D,
  };
  // clang-format on
  return reduce_fmax(array_size(x), x);
}

static inline float onto_E(float const dp[restrict], int const core_size,
                           float const ME, float const DE, float const emis)
{
  // Using lukbak(1) because I'm already in the future (as per make_future)
  // It would be lukbak(0) if I hadn't call make_future on DP_M and DP_D
  float x = DP_M(dp, 0, lukbak(1)) + ME + emis;
  for (int k = 1; k < core_size; ++k)
  {
    x = dcp_fmax(x, DP_M(dp, k, lukbak(1)) + ME + emis);
    x = dcp_fmax(x, DP_D(dp, k, lukbak(1)) + DE + emis);
  }
  return x;
}

static inline float onto_J(float const E[restrict], float const J[restrict],
                           float const EJ, float const JJ,
                           float const emis[restrict])
{
  // clang-format off
  float const x[] = {
      E[lukbak(1)] + EJ + emis[nchars(1)],
      E[lukbak(2)] + EJ + emis[nchars(2)],
      E[lukbak(3)] + EJ + emis[nchars(3)],
      E[lukbak(4)] + EJ + emis[nchars(4)],
      E[lukbak(5)] + EJ + emis[nchars(5)],

      J[lukbak(1)] + JJ + emis[nchars(1)],
      J[lukbak(2)] + JJ + emis[nchars(2)],
      J[lukbak(3)] + JJ + emis[nchars(3)],
      J[lukbak(4)] + JJ + emis[nchars(4)],
      J[lukbak(5)] + JJ + emis[nchars(5)],
  };
  // clang-format on
  return reduce_fmax(array_size(x), x);
}

static inline float onto_C(float const E[restrict], float const C[restrict],
                           float const EC, float const CC,
                           float const emis[restrict])
{
  // clang-format off
  float const x[] = {
      E[lukbak(1)] + EC + emis[nchars(1)],
      E[lukbak(2)] + EC + emis[nchars(2)],
      E[lukbak(3)] + EC + emis[nchars(3)],
      E[lukbak(4)] + EC + emis[nchars(4)],
      E[lukbak(5)] + EC + emis[nchars(5)],

      C[lukbak(1)] + CC + emis[nchars(1)],
      C[lukbak(2)] + CC + emis[nchars(2)],
      C[lukbak(3)] + CC + emis[nchars(3)],
      C[lukbak(4)] + CC + emis[nchars(4)],
      C[lukbak(5)] + CC + emis[nchars(5)],
  };
  // clang-format on
  return reduce_fmax(array_size(x), x);
}

static inline float onto_T(float const E[restrict], float const C[restrict],
                           float const ET, float const CT, float const emis)
{
  float const x[] = {E[lukbak(0)] + ET + emis, C[lukbak(0)] + CT + emis};
  return reduce_fmax(array_size(x), x);
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

DCP_INLINE void make_future(float x[])
{
  memmove(&x[lukbak(1)], &x[lukbak(0)], sizeof(float) * (PAST_SIZE - 1));
}

float dcp_vit_null(struct p7 *x, struct imm_eseq const *eseq)
{
  int seq_size = (int)imm_eseq_size(eseq);
  float const *restrict null_emission = x->null.emission;
  float RR = x->null.RR;

#define NINF IMM_LPROB_ZERO
  float S[PAST_SIZE] = {NINF, NINF, NINF, NINF, NINF, NINF};
  float R[PAST_SIZE] = {NINF, NINF, NINF, NINF, NINF, NINF};
#undef NINF
  S[lukbak(0)] = 0;

  for (int r = 0; r < seq_size + 1; ++r)
  {
    float const null[] = {
        safe_get(null_emission, emission_index(eseq, r - 1, 1, 0), 0),
        safe_get(null_emission, emission_index(eseq, r - 2, 2, 0), 0),
        safe_get(null_emission, emission_index(eseq, r - 3, 3, 0), 0),
        safe_get(null_emission, emission_index(eseq, r - 4, 4, 0), 0),
        safe_get(null_emission, emission_index(eseq, r - 5, 5, 0), 0)};

    R[lukbak(0)] = onto_R(S, R, RR, null);
    make_future(S);
    make_future(R);
    S[lukbak(0)] = IMM_LPROB_ZERO;
  }
  return R[lukbak(0)];
}

DCP_INLINE void vit(struct p7 *x, struct imm_eseq const *eseq, int row_start,
                    int row_end, float dp[restrict], float S[restrict],
                    float N[restrict], float B[restrict], float J[restrict],
                    float E[restrict], float C[restrict], float T[restrict],
                    int const safe)
{
  int core_size = x->core_size;

  float const *restrict BM = x->BMk;
  struct extra_trans const xt = extra_trans(x->xtrans);

  float *restrict emission = x->nodes_emission;
  for (int r = row_start; r < row_end; ++r)
  {
    int ix[5] = {emission_index(eseq, r - 1, 1, safe),
                 emission_index(eseq, r - 2, 2, safe),
                 emission_index(eseq, r - 3, 3, safe),
                 emission_index(eseq, r - 4, 4, safe),
                 emission_index(eseq, r - 5, 5, safe)};

    float const null[] = {safe_get(x->null.emission, ix[nchars(1)], safe),
                          safe_get(x->null.emission, ix[nchars(2)], safe),
                          safe_get(x->null.emission, ix[nchars(3)], safe),
                          safe_get(x->null.emission, ix[nchars(4)], safe),
                          safe_get(x->null.emission, ix[nchars(5)], safe)};

    float const bg[] = {safe_get(x->bg.emission, ix[nchars(1)], safe),
                        safe_get(x->bg.emission, ix[nchars(2)], safe),
                        safe_get(x->bg.emission, ix[nchars(3)], safe),
                        safe_get(x->bg.emission, ix[nchars(4)], safe),
                        safe_get(x->bg.emission, ix[nchars(5)], safe)};

    E[lukbak(0)] = onto_E(dp, core_size, xt.ME, xt.DE, 0);
    N[lukbak(0)] = onto_N(S, N, xt.SN, xt.NN, null);
    B[lukbak(0)] = onto_B(S, N, E, J, xt.SB, xt.NB, xt.EB, xt.JB, 0);
    make_future(S);
    make_future(N);

    float const M[] = {safe_get(emission, ix[nchars(1)], safe),
                       safe_get(emission, ix[nchars(2)], safe),
                       safe_get(emission, ix[nchars(3)], safe),
                       safe_get(emission, ix[nchars(4)], safe),
                       safe_get(emission, ix[nchars(5)], safe)};
    float *DPM = &DP_M(dp, 0, lukbak(0));
    float *DPI = &DP_I(dp, 0, lukbak(0));
    float *DPD = &DP_D(dp, 0, lukbak(0));
    DPM[lukbak(0)] = onto_M1(B, BM[0], M);

    for (int k = 0; k + 1 < core_size; ++k)
    {
      int const k0 = k;
      int const k1 = k + 1;
      struct dcp_trans const *restrict t = &x->nodes[k0].trans;

      float const M[] = {
          safe_get(emission + k1 * P7_NODE_SIZE, ix[nchars(1)], safe),
          safe_get(emission + k1 * P7_NODE_SIZE, ix[nchars(2)], safe),
          safe_get(emission + k1 * P7_NODE_SIZE, ix[nchars(3)], safe),
          safe_get(emission + k1 * P7_NODE_SIZE, ix[nchars(4)], safe),
          safe_get(emission + k1 * P7_NODE_SIZE, ix[nchars(5)], safe)};

      DPI[lukbak(0)] = onto_I(DPM, t->MI, t->II, bg);
      float tmpM = onto_M(DPM, DPI, DPD, B, t->MM, t->IM, t->DM, BM[k1], M);
      __builtin_prefetch(emission + (k1 + 1) * P7_NODE_SIZE + ix[nchars(1)], 0,
                         LOCALITY);
      __builtin_prefetch(emission + (k1 + 1) * P7_NODE_SIZE + ix[nchars(2)], 0,
                         LOCALITY);
      __builtin_prefetch(emission + (k1 + 1) * P7_NODE_SIZE + ix[nchars(3)], 0,
                         LOCALITY);
      __builtin_prefetch(emission + (k1 + 1) * P7_NODE_SIZE + ix[nchars(4)], 0,
                         LOCALITY);
      __builtin_prefetch(emission + (k1 + 1) * P7_NODE_SIZE + ix[nchars(5)], 0,
                         LOCALITY);
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
}

float dcp_vit(struct p7 *x, struct imm_eseq const *eseq)
{
  int core_size = x->core_size;

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

  int seq_size = (int)imm_eseq_size(eseq);
  int row_start = 0;
  int row_mid = seq_size + 1 < 5 ? seq_size + 1 : 5;
  int row_end = seq_size + 1;

  vit(x, eseq, row_start, row_mid, dp, S, N, B, J, E, C, T, 0);
  vit(x, eseq, row_mid, row_end, dp, S, N, B, J, E, C, T, 1);

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
    size_t n = P7_NODE_SIZE;
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
