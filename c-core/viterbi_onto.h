#ifndef VITERBI_ONTO_H
#define VITERBI_ONTO_H

#include "array_size.h"
#include "compiler.h"
#include "find_fmax.h"
#include "imm/imm.h"
#include "reduce_fmax.h"
#include "trellis.h"
#include "viterbi_dp.h"
#include "viterbi_task.h"

CONST int lukbak(int i) { return i; }
CONST int nchars(int n) { return n - 1; }

INLINE float onto_N(struct trellis *t, float const S[restrict],
                    float const N[restrict], float const SN, float const NN,
                    float const emission[restrict])
{
  float const *e = ASSUME_ALIGNED(emission);
  // clang-format off
  float const x[] ALIGNED = {
      S[lukbak(1)] + SN + e[nchars(1)],
      S[lukbak(2)] + SN + e[nchars(2)],
      S[lukbak(3)] + SN + e[nchars(3)],
      S[lukbak(4)] + SN + e[nchars(4)],
      S[lukbak(5)] + SN + e[nchars(5)],

      N[lukbak(1)] + NN + e[nchars(1)],
      N[lukbak(2)] + NN + e[nchars(2)],
      N[lukbak(3)] + NN + e[nchars(3)],
      N[lukbak(4)] + NN + e[nchars(4)],
      N[lukbak(5)] + NN + e[nchars(5)],
  };
  if (!t) return reduce_fmax(array_size(x), x);
  // clang-format on

  int i = find_fmax(array_size(x), x);
  trellis_set(t, STATE_N, i);
  return x[i];
}

INLINE float onto_B(struct trellis *t, float const S[restrict],
                    float const N[restrict], float const SB, float const NB)
{
  // clang-format off
  float const x[] ALIGNED = {
      S[lukbak(0)] + SB + 0,
      N[lukbak(0)] + NB + 0,
  };
  if (!t) return float_maximum(x[0], x[1]);
  // clang-format on

  int i = find_fmax(array_size(x), x);
  trellis_set(t, STATE_B, i);
  return x[i];
}

INLINE float adjust_onto_B(struct trellis *t, float const B[restrict],
                           float const E[restrict], float const J[restrict],
                           float const EB, float const JB)
{
  // clang-format off
  float const x[] ALIGNED = {
      B[lukbak(0)],
      E[lukbak(0)] + EB + 0,
      J[lukbak(0)] + JB + 0,
  };
  if (!t) return reduce_fmax(array_size(x), x);

  int const src[] = {
    -1,
    2,
    3,
  };
  // clang-format on

  int i = find_fmax(array_size(x), x);
  if (i > 0) trellis_replace(t, STATE_B, src[i]);
  return x[i];
}

INLINE float onto_M0(struct trellis *t, float const B[restrict], float const BM,
                     float const emission[restrict])
{
  float const *e = ASSUME_ALIGNED(emission);
  // clang-format off
  float const x[] ALIGNED = {
      B[lukbak(1)] + BM + e[nchars(1)],
      B[lukbak(2)] + BM + e[nchars(2)],
      B[lukbak(3)] + BM + e[nchars(3)],
      B[lukbak(4)] + BM + e[nchars(4)],
      B[lukbak(5)] + BM + e[nchars(5)],
  };
  if (!t) return reduce_fmax(array_size(x), x);
  // clang-format on

  int i = find_fmax(array_size(x), x);
  trellis_set(t, STATE_M, i);
  return x[i];
}

INLINE float onto_I(struct trellis *t, float const M[restrict],
                    float const I[restrict], float const MI, float const II,
                    float const emission[restrict])
{
  float const *e = ASSUME_ALIGNED(emission);
  // clang-format off
  float const x[] ALIGNED = {
      M[lukbak(1)] + MI + e[nchars(1)],
      M[lukbak(2)] + MI + e[nchars(2)],
      M[lukbak(3)] + MI + e[nchars(3)],
      M[lukbak(4)] + MI + e[nchars(4)],
      M[lukbak(5)] + MI + e[nchars(5)],

      I[lukbak(1)] + II + e[nchars(1)],
      I[lukbak(2)] + II + e[nchars(2)],
      I[lukbak(3)] + II + e[nchars(3)],
      I[lukbak(4)] + II + e[nchars(4)],
      I[lukbak(5)] + II + e[nchars(5)],
  };
  if (!t) return reduce_fmax(array_size(x), x);
  // clang-format on

  int i = find_fmax(array_size(x), x);
  trellis_set(t, STATE_I, i);
  return x[i];
}

INLINE float onto_M(struct trellis *t, float const B[restrict],
                    float const M[restrict], float const I[restrict],
                    float const D[restrict], float const BM, float const MM,
                    float const IM, float const DM,
                    float const emission[restrict])
{
  float const *e = ASSUME_ALIGNED(emission);
  // clang-format off
  float const x[] ALIGNED = {
      B[lukbak(1)] + BM + e[nchars(1)],
      B[lukbak(2)] + BM + e[nchars(2)],
      B[lukbak(3)] + BM + e[nchars(3)],
      B[lukbak(4)] + BM + e[nchars(4)],
      B[lukbak(5)] + BM + e[nchars(5)],

      M[lukbak(1)] + MM + e[nchars(1)],
      M[lukbak(2)] + MM + e[nchars(2)],
      M[lukbak(3)] + MM + e[nchars(3)],
      M[lukbak(4)] + MM + e[nchars(4)],
      M[lukbak(5)] + MM + e[nchars(5)],

      I[lukbak(1)] + IM + e[nchars(1)],
      I[lukbak(2)] + IM + e[nchars(2)],
      I[lukbak(3)] + IM + e[nchars(3)],
      I[lukbak(4)] + IM + e[nchars(4)],
      I[lukbak(5)] + IM + e[nchars(5)],

      D[lukbak(1)] + DM + e[nchars(1)],
      D[lukbak(2)] + DM + e[nchars(2)],
      D[lukbak(3)] + DM + e[nchars(3)],
      D[lukbak(4)] + DM + e[nchars(4)],
      D[lukbak(5)] + DM + e[nchars(5)],
  };
  if (!t) return reduce_fmax(array_size(x), x);
  // clang-format on

  int i = find_fmax(array_size(x), x);
  trellis_set(t, STATE_M, i);
  return x[i];
}

INLINE float onto_D(struct trellis *t, float const M[restrict],
                    float const D[restrict], float const MD, float const DD)
{
  // clang-format off
  float const x[] ALIGNED = {
      M[lukbak(0)] + MD + 0,
      D[lukbak(0)] + DD + 0,
  };
  if (!t) return float_maximum(x[0], x[1]);
  // clang-format on

  int i = find_fmax(array_size(x), x);
  trellis_set(t, STATE_D, i);
  return x[i];
}

INLINE void fmax_idx(float *value, int *src, float new_value, int new_src)
{
  if (new_value > *value)
  {
    *value = new_value;
    *src = new_src;
  }
}

INLINE float onto_E(struct trellis *t, float *restrict dp, float const ME,
                    float const DE, int const core_size)
{
  float *Mk = dp_rewind(dp, STATE_M);
  float *Dk = dp_rewind(dp, STATE_D);
  float x = Mk[lukbak(1)] + ME;
  // int src = MIX(0);
  int src = 0;
  for (int i = 2; i < 2 * core_size; i += 2)
  {
    Mk = dp_next(Mk);
    Dk = dp_next(Dk);
    // It is lukbak(1) instead of lukbak(0) because I already called
    // make_future(DPM) and make_future(DPD), for performance reasons.
    fmax_idx(&x, &src, Mk[lukbak(1)] + ME + 0, i + 0);
    fmax_idx(&x, &src, Dk[lukbak(1)] + DE + 0, i + 1);
  }
  trellis_set(t, STATE_E, src);
  return x;
}

INLINE float onto_J(struct trellis *t, float const E[restrict],
                    float const J[restrict], float const EJ, float const JJ,
                    float const emission[restrict])
{
  float const *e = ASSUME_ALIGNED(emission);
  // clang-format off
  float const x[] ALIGNED = {
      E[lukbak(1)] + EJ + e[nchars(1)],
      E[lukbak(2)] + EJ + e[nchars(2)],
      E[lukbak(3)] + EJ + e[nchars(3)],
      E[lukbak(4)] + EJ + e[nchars(4)],
      E[lukbak(5)] + EJ + e[nchars(5)],

      J[lukbak(1)] + JJ + e[nchars(1)],
      J[lukbak(2)] + JJ + e[nchars(2)],
      J[lukbak(3)] + JJ + e[nchars(3)],
      J[lukbak(4)] + JJ + e[nchars(4)],
      J[lukbak(5)] + JJ + e[nchars(5)],
  };
  if (!t) return reduce_fmax(array_size(x), x);
  // clang-format on

  int i = find_fmax(array_size(x), x);
  trellis_set(t, STATE_J, i);
  return x[i];
}

INLINE float onto_C(struct trellis *t, float const E[restrict],
                    float const C[restrict], float const EC, float const CC,
                    float const emission[restrict])
{
  float const *e = ASSUME_ALIGNED(emission);
  // clang-format off
  float const x[] ALIGNED = {
      E[lukbak(1)] + EC + e[nchars(1)],
      E[lukbak(2)] + EC + e[nchars(2)],
      E[lukbak(3)] + EC + e[nchars(3)],
      E[lukbak(4)] + EC + e[nchars(4)],
      E[lukbak(5)] + EC + e[nchars(5)],

      C[lukbak(1)] + CC + e[nchars(1)],
      C[lukbak(2)] + CC + e[nchars(2)],
      C[lukbak(3)] + CC + e[nchars(3)],
      C[lukbak(4)] + CC + e[nchars(4)],
      C[lukbak(5)] + CC + e[nchars(5)],
  };
  if (!t) return reduce_fmax(array_size(x), x);
  // clang-format on

  int i = find_fmax(array_size(x), x);
  trellis_set(t, STATE_C, i);
  return x[i];
}

INLINE float onto_T(struct trellis *t, float const E[restrict],
                    float const C[restrict], float const ET, float const CT)
{
  // clang-format off
  float const x[] ALIGNED = {
    E[lukbak(0)] + ET + 0,
    C[lukbak(0)] + CT + 0,
  };
  if (!t) return float_maximum(x[0], x[1]);
  // clang-format on

  int i = find_fmax(array_size(x), x);
  trellis_set(t, STATE_T, i);
  return x[i];
}

#endif
