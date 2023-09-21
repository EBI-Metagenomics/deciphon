#include "viterbi.h"
#include "array_size.h"
#include "array_size_field.h"
#include "compiler.h"
#include "defer_return.h"
#include "find.h"
#include "imm/imm.h"
#include "protein.h"
#include "reduce.h"
#include "scan_thrd.h"
#include "viterbi_task.h"
#include <stdlib.h>
#include <string.h>

#if __AVX__
#define ALIGNED __attribute__((aligned(32)))
#endif

#if __ARM_NEON
#define ALIGNED __attribute__((aligned(16)))
#endif

#define PAST_SIZE DCP_VITERBI_PAST_SIZE

IMM_CONST int SIDX(void) { return 0; }
IMM_CONST int NIDX(void) { return 1; }
IMM_CONST int BIDX(void) { return 2; }
IMM_CONST int MIDX(int i) { return 3 + i * 3 + 0; }
IMM_CONST int DIDX(int i) { return 3 + i * 3 + 1; }
IMM_CONST int IIDX(int i) { return 3 + i * 3 + 2; }
IMM_CONST int EIDX(int n) { return 3 + n * 3; }
IMM_CONST int JIDX(int n) { return 4 + n * 3; }
IMM_CONST int CIDX(int n) { return 5 + n * 3; }
IMM_CONST int TIDX(int n) { return 6 + n * 3; }

IMM_CONST int CID(int i, int n)
{
  if (i == SIDX()) return STATE_S;
  if (i == NIDX()) return STATE_N;
  if (i == BIDX()) return STATE_B;
  if (i < EIDX(n))
  {
    int core_idx = (i - 3) / 3;
    if (i % 3 == 0) return dcp_state_make_match_id(core_idx);
    if (i % 3 == 1) return dcp_state_make_delete_id(core_idx);
    if (i % 3 == 2) return dcp_state_make_insert_id(core_idx);
    assert(0);
  }
  if (i == EIDX(n)) return STATE_E;
  if (i == JIDX(n)) return STATE_J;
  if (i == CIDX(n)) return STATE_C;
  if (i == TIDX(n)) return STATE_T;
  assert(0);
  return 0;
}

#define lukbak(i) ((i))
#define nchars(n) ((n)-1)

IMM_CONST float *dp_match_init(float *x) { return x; }
IMM_CONST float *dp_insert_init(float *x) { return x + PAST_SIZE; }
IMM_CONST float *dp_delete_init(float *x) { return x + PAST_SIZE + PAST_SIZE; }
IMM_CONST float *dp_next(float *x) { return x + 3 * PAST_SIZE; }

DCP_PURE int emission_index(struct imm_eseq const *x, int pos, int size,
                            bool const safe)
{
  return ((!safe && (pos) < 0) ? -1 : (int)imm_eseq_get(x, pos, size, 1));
}

DCP_PURE float safe_get(float const x[restrict], int i, bool const safe)
{
  return (safe ? (x)[(i)] : (i) >= 0 ? (x)[(i)] : IMM_LPROB_ZERO);
}

DCP_PURE float const *unsafe_get(float const x[restrict], int i)
{
  return x + i;
}

DCP_INLINE void fetch_indices(int x[restrict], struct imm_eseq const *eseq,
                              int row, bool const safe)
{
  x[0] = emission_index(eseq, row - 1, 1, safe);
  x[1] = emission_index(eseq, row - 2, 2, safe);
  x[2] = emission_index(eseq, row - 3, 3, safe);
  x[3] = emission_index(eseq, row - 4, 4, safe);
  x[4] = emission_index(eseq, row - 5, 5, safe);
}

DCP_INLINE void fetch_emission(float x[restrict], float emission[restrict],
                               int const ix[restrict], bool const safe)
{
  x[0] = safe_get(emission, ix[nchars(1)], safe);
  x[1] = safe_get(emission, ix[nchars(2)], safe);
  x[2] = safe_get(emission, ix[nchars(3)], safe);
  x[3] = safe_get(emission, ix[nchars(4)], safe);
  x[4] = safe_get(emission, ix[nchars(5)], safe);
}

DCP_INLINE void prefetch_emission(float emission[restrict],
                                  int const ix[restrict])
{
  DCP_PREFETCH(unsafe_get(emission, ix[nchars(1)]), 0, 1);
  DCP_PREFETCH(unsafe_get(emission, ix[nchars(2)]), 0, 1);
  DCP_PREFETCH(unsafe_get(emission, ix[nchars(3)]), 0, 1);
  DCP_PREFETCH(unsafe_get(emission, ix[nchars(4)]), 0, 1);
  DCP_PREFETCH(unsafe_get(emission, ix[nchars(5)]), 0, 1);
}

DCP_CONST float *match_next(float *restrict match)
{
  return match + PROTEIN_NODE_SIZE;
}

DCP_INLINE void update_trellis(struct imm_trellis *t, int const src[restrict],
                               int size, float const x[restrict],
                               struct imm_span span)
{
  int idx = find_fmax(size, x);
  imm_trellis_push(t, x[idx], src[idx], idx % span.max + span.min);
}

DCP_INLINE void update_trellis0(struct imm_trellis *t, int const src[restrict],
                                int size, float const x[restrict])
{
  int idx = find_fmax(size, x);
  imm_trellis_push(t, x[idx], src[idx], 0);
}

DCP_PURE float onto_R(float const S[restrict], float const R[restrict],
                      float const RR, float const emis[restrict])
{
  // clang-format off
  float const x[] ALIGNED = {
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

DCP_INLINE float onto_N(struct imm_trellis *t, float const S[restrict],
                        float const N[restrict], float const SN, float const NN,
                        float const emis[restrict])
{
  int const src[] = {SIDX(), SIDX(), SIDX(), SIDX(), SIDX(),
                     NIDX(), NIDX(), NIDX(), NIDX(), NIDX()};
  // clang-format off
  float const x[] ALIGNED = {
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
  if (t) update_trellis(t, src, array_size(x), x, imm_span(1, 5));
  return reduce_fmax(array_size(x), x);
}

DCP_INLINE float onto_B(struct imm_trellis *t, float const S[restrict],
                        float const N[restrict], float const SB, float const NB)
{
  int const src[] = {SIDX(), NIDX()};
  // clang-format off
  float const x[] ALIGNED = {
      S[lukbak(0)] + SB,
      N[lukbak(0)] + NB,
  };
  // clang-format on
  if (t) update_trellis0(t, src, array_size(x), x);
  return dcp_fmax(x[0], x[1]);
}

DCP_INLINE void adjust_onto_B(struct imm_trellis *t, float B[restrict],
                              float const E[restrict], float const J[restrict],
                              float const EB, float const JB, int core_size)
{
  int const src[] = {EIDX(core_size), JIDX(core_size)};
  // clang-format off
  float const x[] ALIGNED = {
      E[lukbak(0)] + EB,
      J[lukbak(0)] + JB,
  };
  // clang-format on
  if (t)
  {
    int stage = imm_trellis_stage_idx(t);
    imm_trellis_seek(t, stage, BIDX());
    assert((int)imm_trellis_state_idx(t) == BIDX());
    int idx = find_fmax(array_size(x), x);
    if (x[idx] > B[lukbak(0)]) imm_trellis_push(t, x[idx], src[idx], 0);
    imm_trellis_seek(t, stage, CIDX(core_size));
  }
  B[lukbak(0)] = dcp_fmax(B[lukbak(0)], reduce_fmax(array_size(x), x));
}

DCP_INLINE float onto_M1(struct imm_trellis *t, float const B[restrict],
                         float const BM, float const emis[restrict])
{
  int const src[] = {BIDX(), BIDX(), BIDX(), BIDX(), BIDX()};
  // clang-format off
  float const x[] ALIGNED = {
      B[lukbak(1)] + BM + emis[nchars(1)],
      B[lukbak(2)] + BM + emis[nchars(2)],
      B[lukbak(3)] + BM + emis[nchars(3)],
      B[lukbak(4)] + BM + emis[nchars(4)],
      B[lukbak(5)] + BM + emis[nchars(5)],
  };
  // clang-format on
  if (t) update_trellis(t, src, array_size(x), x, imm_span(1, 5));
  return reduce_fmax(array_size(x), x);
}

DCP_INLINE float onto_M(struct imm_trellis *t, float const M[restrict],
                        float const I[restrict], float const D[restrict],
                        float const B[restrict], float const MM, float const IM,
                        float const DM, float const BM,
                        float const emis[restrict], int k)
{
  int const src[] = {BIDX(),  BIDX(),  BIDX(),  BIDX(),  BIDX(),
                     MIDX(k), MIDX(k), MIDX(k), MIDX(k), MIDX(k),
                     IIDX(k), IIDX(k), IIDX(k), IIDX(k), IIDX(k),
                     DIDX(k), DIDX(k), DIDX(k), DIDX(k), DIDX(k)};
  // clang-format off
  float const x[] ALIGNED = {
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
  if (t) update_trellis(t, src, array_size(x), x, imm_span(1, 5));
  return reduce_fmax(array_size(x), x);
}

DCP_INLINE float onto_I(struct imm_trellis *t, float const M[restrict],
                        float const I[restrict], float const MI, float const II,
                        float const emis[restrict], int k)
{
  int const src[] = {MIDX(k), MIDX(k), MIDX(k), MIDX(k), MIDX(k),
                     IIDX(k), IIDX(k), IIDX(k), IIDX(k), IIDX(k)};
  // clang-format off
  float const x[] ALIGNED = {
      M[lukbak(1)] + MI + emis[nchars(1)],
      M[lukbak(2)] + MI + emis[nchars(2)],
      M[lukbak(3)] + MI + emis[nchars(3)],
      M[lukbak(4)] + MI + emis[nchars(4)],
      M[lukbak(5)] + MI + emis[nchars(5)],

      I[lukbak(1)] + II + emis[nchars(1)],
      I[lukbak(2)] + II + emis[nchars(2)],
      I[lukbak(3)] + II + emis[nchars(3)],
      I[lukbak(4)] + II + emis[nchars(4)],
      I[lukbak(5)] + II + emis[nchars(5)],
  };
  // clang-format on
  if (t) update_trellis(t, src, array_size(x), x, imm_span(1, 5));
  return reduce_fmax(array_size(x), x);
}

DCP_INLINE float onto_D(struct imm_trellis *t, float const M[restrict],
                        float const D[restrict], float const MD, float const DD,
                        int k)
{
  int const src[] = {MIDX(k), DIDX(k)};
  // clang-format off
  float const x[] ALIGNED = {
      M[lukbak(0)] + MD,
      D[lukbak(0)] + DD,
  };
  // clang-format on
  if (t) update_trellis0(t, src, array_size(x), x);
  return dcp_fmax(x[0], x[1]);
}

DCP_INLINE float fmax_idx(float max, int *src, int *maxk, float v, int new_src,
                          int new_maxk)
{
  if (v > max)
  {
    max = v;
    *src = new_src;
    *maxk = new_maxk;
  }
  return max;
}

IMM_INLINE float onto_E(struct imm_trellis *t, float const dp[restrict],
                        int const core_size, float const ME, float const DE)
{
  float const *DPM = dp_match_init((float *)dp);
  float const *DPD = dp_delete_init((float *)dp);
  float x = DPM[lukbak(1)] + ME;
  int src = MIDX(0);
  int maxk = 0;
  for (int k = 1; k < core_size; ++k)
  {
    DPM = dp_next((float *)DPM);
    DPD = dp_next((float *)DPD);
    if (t)
    {
      x = fmax_idx(x, &src, &maxk, DPM[lukbak(1)] + ME, MIDX(k), k);
      x = fmax_idx(x, &src, &maxk, DPD[lukbak(1)] + DE, DIDX(k), k);
    }
    else
    {
      x = dcp_fmax(x, DPM[lukbak(1)] + ME);
      x = dcp_fmax(x, DPD[lukbak(1)] + DE);
    }
  }
  if (t) imm_trellis_push(t, x, src, 0);
  return x;
}

IMM_INLINE float onto_J(struct imm_trellis *t, float const E[restrict],
                        float const J[restrict], float const EJ, float const JJ,
                        float const emis[restrict], int core_size)
{
  int const src[] = {EIDX(core_size), EIDX(core_size), EIDX(core_size),
                     EIDX(core_size), EIDX(core_size), JIDX(core_size),
                     JIDX(core_size), JIDX(core_size), JIDX(core_size),
                     JIDX(core_size)};
  // clang-format off
  float const x[] ALIGNED = {
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
  if (t) update_trellis(t, src, array_size(x), x, imm_span(1, 5));
  return reduce_fmax(array_size(x), x);
}

IMM_INLINE float onto_C(struct imm_trellis *t, float const E[restrict],
                        float const C[restrict], float const EC, float const CC,
                        float const emis[restrict], int core_size)
{
  int const src[] = {EIDX(core_size), EIDX(core_size), EIDX(core_size),
                     EIDX(core_size), EIDX(core_size), CIDX(core_size),
                     CIDX(core_size), CIDX(core_size), CIDX(core_size),
                     CIDX(core_size)};
  // clang-format off
  float const x[] ALIGNED = {
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
  if (t) update_trellis(t, src, array_size(x), x, imm_span(1, 5));
  return reduce_fmax(array_size(x), x);
}

IMM_INLINE float onto_T(struct imm_trellis *t, float const E[restrict],
                        float const C[restrict], float const ET, float const CT,
                        int core_size)
{
  int const src[] = {EIDX(core_size), CIDX(core_size)};
  // clang-format off
  float const x[] ALIGNED = {
    E[lukbak(0)] + ET,
    C[lukbak(0)] + CT,
  };
  // clang-format on
  if (t) update_trellis0(t, src, array_size(x), x);
  return dcp_fmax(x[0], x[1]);
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

float dcp_viterbi_null(struct dcp_protein *x, struct imm_eseq const *eseq)
{
  int seq_size = (int)imm_eseq_size(eseq);

#define NINF IMM_LPROB_ZERO
  float S[PAST_SIZE] = {NINF, NINF, NINF, NINF, NINF, NINF};
  float R[PAST_SIZE] = {NINF, NINF, NINF, NINF, NINF, NINF};
#undef NINF
  S[lukbak(0)] = 0;

  int ix[PAST_SIZE - 1] = {0};
  float null[PAST_SIZE - 1] = {0};
  for (int r = 0; r < seq_size + 1; ++r)
  {
    fetch_indices(ix, eseq, r, false);
    fetch_emission(null, x->null.emission, ix, false);

    R[lukbak(0)] = onto_R(S, R, x->null.RR, null);
    make_future(S);
    make_future(R);
    S[lukbak(0)] = IMM_LPROB_ZERO;
  }
  return R[lukbak(0)];
}

DCP_INLINE void vit(struct dcp_protein *x, struct dcp_viterbi_task *task,
                    struct imm_eseq const *eseq, int row_start, int row_end,
                    bool const safe, struct imm_trellis *trellis)
{
  int core_size = x->core_size;

  struct extra_trans const xt = extra_trans(x->xtrans);
  float *dp = task->dp;
  float *restrict S = task->S;
  float *restrict N = task->N;
  float *restrict B = task->B;
  float *restrict J = task->J;
  float *restrict E = task->E;
  float *restrict C = task->C;
  float *restrict T = task->T;

  int ix[PAST_SIZE - 1] = {0};
  float null[PAST_SIZE - 1] = {0};
  float bg[PAST_SIZE - 1] = {0};
  float M[PAST_SIZE - 1] = {0};

  for (int r = row_start; r < row_end; ++r)
  {
    if (trellis) imm_trellis_seek(trellis, r, NIDX());
    float const *restrict BM = x->BMk;
    float *restrict match = x->nodes_emission;
    fetch_indices(ix, eseq, r, safe);
    fetch_emission(null, x->null.emission, ix, safe);
    fetch_emission(bg, x->bg.emission, ix, safe);

    N[lukbak(0)] = onto_N(trellis, S, N, xt.SN, xt.NN, null);
    B[lukbak(0)] = onto_B(trellis, S, N, xt.SB, xt.NB);
    make_future(S);
    make_future(N);

    fetch_emission(M, match, ix, safe);
    float *DPM = dp_match_init(dp);
    float *DPI = dp_insert_init(dp);
    float *DPD = dp_delete_init(dp);
    DPM[lukbak(0)] = onto_M1(trellis, B, *BM, M);
    float tmpE = DPM[lukbak(0)] + xt.ME;
    BM += 1;
    // Skip first D state
    if (trellis) trellis->head += 1;

    struct dcp_protein_node const *node = x->nodes;
    for (int k = 0; k + 1 < core_size; ++k)
    {
      float const MM = node->trans.MM;
      float const MI = node->trans.MI;
      float const MD = node->trans.MD;
      float const IM = node->trans.IM;
      float const II = node->trans.II;
      float const DM = node->trans.DM;
      float const DD = node->trans.DD;

      match = match_next(match);
      fetch_emission(M, match, ix, safe);

      DPI[lukbak(0)] = onto_I(trellis, DPM, DPI, MI, II, bg, k);
      float tmpM = onto_M(trellis, DPM, DPI, DPD, B, MM, IM, DM, *BM, M, k);
      prefetch_emission(match_next(match), ix);
      float tmpD = onto_D(trellis, DPM, DPD, MD, DD, k);
      make_future(DPM);
      make_future(DPI);
      make_future(DPD);
      DPM = dp_next(DPM);
      DPM[lukbak(0)] = tmpM;
      tmpE = dcp_fmax(tmpE, tmpM + xt.ME);
      DPI = dp_next(DPI);
      DPD = dp_next(DPD);
      DPD[lukbak(0)] = tmpD;
      tmpE = dcp_fmax(tmpE, tmpD + xt.DE);
      BM += 1;
      node += 1;
    }
    // Skip last I state
    if (trellis) trellis->head += 1;
    make_future(DPM);
    make_future(DPI);
    make_future(DPD);

    // It is lukbak(1) in here because I called make_future(DPM)
    // and make_future(DPD) already (for optimisation reasons).
    if (trellis)
      E[lukbak(0)] = onto_E(trellis, dp, core_size, xt.ME, xt.DE);
    else
      E[lukbak(0)] = tmpE;
    J[lukbak(0)] = onto_J(trellis, E, J, xt.EJ, xt.JJ, null, core_size);
    adjust_onto_B(trellis, B, E, J, xt.EB, xt.JB, core_size);
    make_future(B);
    make_future(J);
    C[lukbak(0)] = onto_C(trellis, E, C, xt.EC, xt.CC, null, core_size);
    T[lukbak(0)] = onto_T(trellis, E, C, xt.ET, xt.CT, core_size);
    make_future(E);
    make_future(C);
    make_future(T);
    S[lukbak(0)] = IMM_LPROB_ZERO;
  }
}

static void unzip_path(struct imm_trellis *x, int core_size, unsigned seq_size,
                       struct imm_path *path);

int dcp_viterbi(struct dcp_protein *x, struct imm_eseq const *eseq,
                struct dcp_viterbi_task *task, bool const nopath)
{
  int seq_size = (int)imm_eseq_size(eseq);
  int rc = dcp_viterbi_task_setup(task, x->core_size, seq_size, nopath);
  if (rc) return rc;

  task->S[lukbak(0)] = 0;

  int row_start = 0;
  int row_mid = seq_size + 1 < 5 ? seq_size + 1 : 5;
  int row_end = seq_size + 1;

  if (nopath)
  {
    vit(x, task, eseq, row_start, row_mid, false, NULL);
    vit(x, task, eseq, row_mid, row_end, true, NULL);
  }
  else
  {
    vit(x, task, eseq, row_start, row_mid, false, &task->trellis);
    vit(x, task, eseq, row_mid, row_end, true, &task->trellis);
    imm_path_reset(&task->path);
    unzip_path(&task->trellis, x->core_size, seq_size, &task->path);
  }

  task->score = task->T[lukbak(1)];
  return rc;
}

static void unzip_path(struct imm_trellis *x, int core_size, unsigned seq_size,
                       struct imm_path *path)
{
  unsigned start_state = SIDX();
  unsigned end_state = TIDX(core_size);
  imm_trellis_seek(x, seq_size, end_state);
  if (imm_lprob_is_nan(imm_trellis_head(x)->score)) return;

  while (imm_trellis_state_idx(x) != start_state || imm_trellis_stage_idx(x))
  {
    unsigned size = imm_trellis_head(x)->emission_size;
    unsigned id = CID(imm_trellis_state_idx(x), core_size);
    float score = imm_trellis_head(x)->score;
    imm_path_add(path, imm_step(id, size, score));
    imm_trellis_back(x);
  }
  unsigned id = CID(imm_trellis_state_idx(x), core_size);
  imm_path_add(path, imm_step(id, 0, 0));
  imm_path_reverse(path);
}

void dcp_viterbi_dump(struct dcp_protein *x, FILE *restrict fp)
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
  size_t bg_size = array_size_field(struct dcp_protein_background, emission);

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
    size_t n = PROTEIN_NODE_SIZE;
    imm_dump_array_f32(n, match_emission, fp);
    fputc('\n', fp);
  }
}

void dcp_viterbi_dump_dot(struct dcp_protein *x, FILE *restrict fp)
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
