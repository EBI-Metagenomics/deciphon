#ifndef DECIPHON_VITERBI_ONTO_H
#define DECIPHON_VITERBI_ONTO_H

#include "array_size.h"
#include "compiler.h"
#include "find.h"
#include "imm/imm.h"
#include "reduce.h"
#include "viterbi_index.h"
#include "viterbi_task.h"

DCP_INLINE float onto_N(struct imm_trellis *t, float const S[restrict],
                        float const N[restrict], float const SN, float const NN,
                        float const emission[restrict])
{
  // clang-format off
  float const x[] ALIGNED = {
      S[lukbak(1)] + SN + emission[nchars(1)],
      S[lukbak(2)] + SN + emission[nchars(2)],
      S[lukbak(3)] + SN + emission[nchars(3)],
      S[lukbak(4)] + SN + emission[nchars(4)],
      S[lukbak(5)] + SN + emission[nchars(5)],

      N[lukbak(1)] + NN + emission[nchars(1)],
      N[lukbak(2)] + NN + emission[nchars(2)],
      N[lukbak(3)] + NN + emission[nchars(3)],
      N[lukbak(4)] + NN + emission[nchars(4)],
      N[lukbak(5)] + NN + emission[nchars(5)],
  };
  if (!t) return reduce_fmax(array_size(x), x);

  int const src[] = {
    SIX(),
    SIX(),
    SIX(),
    SIX(),
    SIX(),

    NIX(),
    NIX(),
    NIX(),
    NIX(),
    NIX(),
  };
  // clang-format on

  int i = find_fmax(array_size(x), x);
  imm_trellis_push(t, x[i], src[i], i % 5 + 1);
  return x[i];
}

DCP_INLINE float onto_B(struct imm_trellis *t, float const S[restrict],
                        float const N[restrict], float const SB, float const NB)
{
  // clang-format off
  float const x[] ALIGNED = {
      S[lukbak(0)] + SB + 0,
      N[lukbak(0)] + NB + 0,
  };
  if (!t) return dcp_fmax(x[0], x[1]);

  int const src[] = {
    SIX(),
    NIX(),
  };
  // clang-format on

  int i = find_fmax(array_size(x), x);
  imm_trellis_push(t, x[i], src[i], 0);
  return x[i];
}

DCP_INLINE float adjust_onto_B(struct imm_trellis *t, float const B[restrict],
                               float const E[restrict], float const J[restrict],
                               float const EB, float const JB,
                               int const core_size)
{
  // clang-format off
  float const x[] ALIGNED = {
      B[lukbak(0)],
      E[lukbak(0)] + EB + 0,
      J[lukbak(0)] + JB + 0,
  };
  if (!t) return reduce_fmax(array_size(x), x);

  int const src[] = {
    imm_trellis_head(t)->state_source,
    EIX(core_size),
    JIX(core_size),
  };
  // clang-format on

  int i = find_fmax(array_size(x), x);
  imm_trellis_push(t, x[i], src[i], 0);
  return x[i];
}

DCP_INLINE float onto_M0(struct imm_trellis *t, float const B[restrict],
                         float const BM, float const emission[restrict])
{
  // clang-format off
  float const x[] ALIGNED = {
      B[lukbak(1)] + BM + emission[nchars(1)],
      B[lukbak(2)] + BM + emission[nchars(2)],
      B[lukbak(3)] + BM + emission[nchars(3)],
      B[lukbak(4)] + BM + emission[nchars(4)],
      B[lukbak(5)] + BM + emission[nchars(5)],
  };
  if (!t) return reduce_fmax(array_size(x), x);

  int const src[] = {
    BIX(),
    BIX(),
    BIX(),
    BIX(),
    BIX(),
  };
  // clang-format on

  int i = find_fmax(array_size(x), x);
  imm_trellis_push(t, x[i], src[i], i % 5 + 1);
  return x[i];
}

DCP_INLINE float onto_I(struct imm_trellis *t, float const M[restrict],
                        float const I[restrict], float const MI, float const II,
                        float const emission[restrict], int k)
{
  // clang-format off
  float const x[] ALIGNED = {
      M[lukbak(1)] + MI + emission[nchars(1)],
      M[lukbak(2)] + MI + emission[nchars(2)],
      M[lukbak(3)] + MI + emission[nchars(3)],
      M[lukbak(4)] + MI + emission[nchars(4)],
      M[lukbak(5)] + MI + emission[nchars(5)],

      I[lukbak(1)] + II + emission[nchars(1)],
      I[lukbak(2)] + II + emission[nchars(2)],
      I[lukbak(3)] + II + emission[nchars(3)],
      I[lukbak(4)] + II + emission[nchars(4)],
      I[lukbak(5)] + II + emission[nchars(5)],
  };
  if (!t) return reduce_fmax(array_size(x), x);

  int const src[] = {
    MIX(k),
    MIX(k),
    MIX(k),
    MIX(k),
    MIX(k),

    IIX(k),
    IIX(k),
    IIX(k),
    IIX(k),
    IIX(k),
  };
  // clang-format on

  int i = find_fmax(array_size(x), x);
  imm_trellis_push(t, x[i], src[i], i % 5 + 1);
  return x[i];
}

DCP_INLINE float onto_M(struct imm_trellis *t, float const B[restrict],
                        float const M[restrict], float const I[restrict],
                        float const D[restrict], float const BM, float const MM,
                        float const IM, float const DM,
                        float const emission[restrict], int k)
{
  // clang-format off
  float const x[] ALIGNED = {
      B[lukbak(1)] + BM + emission[nchars(1)],
      B[lukbak(2)] + BM + emission[nchars(2)],
      B[lukbak(3)] + BM + emission[nchars(3)],
      B[lukbak(4)] + BM + emission[nchars(4)],
      B[lukbak(5)] + BM + emission[nchars(5)],

      M[lukbak(1)] + MM + emission[nchars(1)],
      M[lukbak(2)] + MM + emission[nchars(2)],
      M[lukbak(3)] + MM + emission[nchars(3)],
      M[lukbak(4)] + MM + emission[nchars(4)],
      M[lukbak(5)] + MM + emission[nchars(5)],

      I[lukbak(1)] + IM + emission[nchars(1)],
      I[lukbak(2)] + IM + emission[nchars(2)],
      I[lukbak(3)] + IM + emission[nchars(3)],
      I[lukbak(4)] + IM + emission[nchars(4)],
      I[lukbak(5)] + IM + emission[nchars(5)],

      D[lukbak(1)] + DM + emission[nchars(1)],
      D[lukbak(2)] + DM + emission[nchars(2)],
      D[lukbak(3)] + DM + emission[nchars(3)],
      D[lukbak(4)] + DM + emission[nchars(4)],
      D[lukbak(5)] + DM + emission[nchars(5)],
  };
  if (!t) return reduce_fmax(array_size(x), x);

  int const src[] = {
    BIX(),
    BIX(),
    BIX(),
    BIX(),
    BIX(),

    MIX(k),
    MIX(k),
    MIX(k),
    MIX(k),
    MIX(k),

    IIX(k),
    IIX(k),
    IIX(k),
    IIX(k),
    IIX(k),

    DIX(k),
    DIX(k),
    DIX(k),
    DIX(k),
    DIX(k),
  };
  // clang-format on

  int i = find_fmax(array_size(x), x);
  imm_trellis_push(t, x[i], src[i], i % 5 + 1);
  return x[i];
}

DCP_INLINE float onto_D(struct imm_trellis *t, float const M[restrict],
                        float const D[restrict], float const MD, float const DD,
                        int k)
{
  // clang-format off
  float const x[] ALIGNED = {
      M[lukbak(0)] + MD + 0,
      D[lukbak(0)] + DD + 0,
  };
  if (!t) return dcp_fmax(x[0], x[1]);

  int const src[] = {
    MIX(k),
    DIX(k),
  };
  // clang-format on

  int i = find_fmax(array_size(x), x);
  imm_trellis_push(t, x[i], src[i], 0);
  return x[i];
}

DCP_INLINE void fmax_idx(float *value, int *src, float new_value, int new_src)
{
  if (new_value > *value)
  {
    *value = new_value;
    *src = new_src;
  }
}

DCP_INLINE float onto_E(struct imm_trellis *t, float *restrict dp,
                        float const ME, float const DE, int const core_size)
{
  float *Mk = dp_rewind(dp, STATE_MATCH);
  float *Dk = dp_rewind(dp, STATE_DELETE);
  float x = Mk[lukbak(1)] + ME;
  int src = MIX(0);
  for (int k = 1; k < core_size; ++k)
  {
    Mk = dp_next(Mk);
    Dk = dp_next(Dk);
    // It is lukbak(1) instead of lukbak(0) because I already called
    // make_future(DPM) and make_future(DPD), for performance reasons.
    fmax_idx(&x, &src, Mk[lukbak(1)] + ME + 0, MIX(k));
    fmax_idx(&x, &src, Dk[lukbak(1)] + DE + 0, DIX(k));
  }
  imm_trellis_push(t, x, src, 0);
  return x;
}

DCP_INLINE float onto_J(struct imm_trellis *t, float const E[restrict],
                        float const J[restrict], float const EJ, float const JJ,
                        float const emission[restrict], int const core_size)
{
  // clang-format off
  float const x[] ALIGNED = {
      E[lukbak(1)] + EJ + emission[nchars(1)],
      E[lukbak(2)] + EJ + emission[nchars(2)],
      E[lukbak(3)] + EJ + emission[nchars(3)],
      E[lukbak(4)] + EJ + emission[nchars(4)],
      E[lukbak(5)] + EJ + emission[nchars(5)],

      J[lukbak(1)] + JJ + emission[nchars(1)],
      J[lukbak(2)] + JJ + emission[nchars(2)],
      J[lukbak(3)] + JJ + emission[nchars(3)],
      J[lukbak(4)] + JJ + emission[nchars(4)],
      J[lukbak(5)] + JJ + emission[nchars(5)],
  };
  if (!t) return reduce_fmax(array_size(x), x);

  int const src[] = {
    EIX(core_size),
    EIX(core_size),
    EIX(core_size),
    EIX(core_size),
    EIX(core_size),

    JIX(core_size),
    JIX(core_size),
    JIX(core_size),
    JIX(core_size),
    JIX(core_size),
  };
  // clang-format on

  int i = find_fmax(array_size(x), x);
  imm_trellis_push(t, x[i], src[i], i % 5 + 1);
  return x[i];
}

DCP_INLINE float onto_C(struct imm_trellis *t, float const E[restrict],
                        float const C[restrict], float const EC, float const CC,
                        float const emission[restrict], int const core_size)
{
  // clang-format off
  float const x[] ALIGNED = {
      E[lukbak(1)] + EC + emission[nchars(1)],
      E[lukbak(2)] + EC + emission[nchars(2)],
      E[lukbak(3)] + EC + emission[nchars(3)],
      E[lukbak(4)] + EC + emission[nchars(4)],
      E[lukbak(5)] + EC + emission[nchars(5)],

      C[lukbak(1)] + CC + emission[nchars(1)],
      C[lukbak(2)] + CC + emission[nchars(2)],
      C[lukbak(3)] + CC + emission[nchars(3)],
      C[lukbak(4)] + CC + emission[nchars(4)],
      C[lukbak(5)] + CC + emission[nchars(5)],
  };
  if (!t) return reduce_fmax(array_size(x), x);

  int const src[] = {
    EIX(core_size),
    EIX(core_size),
    EIX(core_size),
    EIX(core_size),
    EIX(core_size),

    CIX(core_size),
    CIX(core_size),
    CIX(core_size),
    CIX(core_size),
    CIX(core_size),
  };
  // clang-format on

  int i = find_fmax(array_size(x), x);
  imm_trellis_push(t, x[i], src[i], i % 5 + 1);
  return x[i];
}

DCP_INLINE float onto_T(struct imm_trellis *t, float const E[restrict],
                        float const C[restrict], float const ET, float const CT,
                        int const core_size)
{
  // clang-format off
  float const x[] ALIGNED = {
    E[lukbak(0)] + ET + 0,
    C[lukbak(0)] + CT + 0,
  };
  if (!t) return dcp_fmax(x[0], x[1]);

  int const src[] = {
    EIX(core_size),
    CIX(core_size),
  };
  // clang-format on

  int i = find_fmax(array_size(x), x);
  imm_trellis_push(t, x[i], src[i], 0);
  return x[i];
}

#endif
