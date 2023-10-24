#ifndef VITERBI_ONTO_H
#define VITERBI_ONTO_H

#include "array_size.h"
#include "compiler.h"
#include "find_fmax.h"
#include "imm/imm.h"
#include "reduce_fmax.h"
#include "trellis.h"
#include "viterbi_dp.h"
#include "viterbi_emission.h"

PURE float onto_R(float const S[restrict], float const R[restrict],
                  float const RR, float const e[restrict])
{
  // clang-format off
  float const x[] ALIGNED = {
      dp_get(S, 1) + 0 + emission_of(e, 1),
      dp_get(S, 2) + 0 + emission_of(e, 2),
      dp_get(S, 3) + 0 + emission_of(e, 3),
      dp_get(S, 4) + 0 + emission_of(e, 4),
      dp_get(S, 5) + 0 + emission_of(e, 5),

      dp_get(R, 1) + RR + emission_of(e, 1),
      dp_get(R, 2) + RR + emission_of(e, 2),
      dp_get(R, 3) + RR + emission_of(e, 3),
      dp_get(R, 4) + RR + emission_of(e, 4),
      dp_get(R, 5) + RR + emission_of(e, 5),
  };
  // clang-format on
  return reduce_fmax(array_size(x), x);
}

INLINE float onto_N(struct trellis *t, float const S[restrict],
                    float const N[restrict], float const SN, float const NN,
                    float const e[restrict])
{
  // clang-format off
  float const x[] ALIGNED = {
      dp_get(S, 1) + SN + emission_of(e, 1),
      dp_get(S, 2) + SN + emission_of(e, 2),
      dp_get(S, 3) + SN + emission_of(e, 3),
      dp_get(S, 4) + SN + emission_of(e, 4),
      dp_get(S, 5) + SN + emission_of(e, 5),

      dp_get(N, 1) + NN + emission_of(e, 1),
      dp_get(N, 2) + NN + emission_of(e, 2),
      dp_get(N, 3) + NN + emission_of(e, 3),
      dp_get(N, 4) + NN + emission_of(e, 4),
      dp_get(N, 5) + NN + emission_of(e, 5),
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
      dp_get(S, 0) + SB + 0,
      dp_get(N, 0) + NB + 0,
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
      dp_get(B, 0),
      dp_get(E, 0) + EB + 0,
      dp_get(J, 0) + JB + 0,
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
                     float const e[restrict])
{
  // clang-format off
  float const x[] ALIGNED = {
      dp_get(B, 1) + BM + emission_of(e, 1),
      dp_get(B, 2) + BM + emission_of(e, 2),
      dp_get(B, 3) + BM + emission_of(e, 3),
      dp_get(B, 4) + BM + emission_of(e, 4),
      dp_get(B, 5) + BM + emission_of(e, 5),
  };
  if (!t) return reduce_fmax(array_size(x), x);
  // clang-format on

  int i = find_fmax(array_size(x), x);
  trellis_set(t, STATE_M, i);
  return x[i];
}

INLINE float onto_I(struct trellis *t, float const M[restrict],
                    float const I[restrict], float const MI, float const II,
                    float const e[restrict])
{
  // clang-format off
  float const x[] ALIGNED = {
      dp_get(M, 1) + MI + emission_of(e, 1),
      dp_get(M, 2) + MI + emission_of(e, 2),
      dp_get(M, 3) + MI + emission_of(e, 3),
      dp_get(M, 4) + MI + emission_of(e, 4),
      dp_get(M, 5) + MI + emission_of(e, 5),

      dp_get(I, 1) + II + emission_of(e, 1),
      dp_get(I, 2) + II + emission_of(e, 2),
      dp_get(I, 3) + II + emission_of(e, 3),
      dp_get(I, 4) + II + emission_of(e, 4),
      dp_get(I, 5) + II + emission_of(e, 5),
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
                    float const IM, float const DM, float const e[restrict])
{
  // clang-format off
  float const x[] ALIGNED = {
      dp_get(B, 1) + BM + emission_of(e, 1),
      dp_get(B, 2) + BM + emission_of(e, 2),
      dp_get(B, 3) + BM + emission_of(e, 3),
      dp_get(B, 4) + BM + emission_of(e, 4),
      dp_get(B, 5) + BM + emission_of(e, 5),

      dp_get(M, 1) + MM + emission_of(e, 1),
      dp_get(M, 2) + MM + emission_of(e, 2),
      dp_get(M, 3) + MM + emission_of(e, 3),
      dp_get(M, 4) + MM + emission_of(e, 4),
      dp_get(M, 5) + MM + emission_of(e, 5),

      dp_get(I, 1) + IM + emission_of(e, 1),
      dp_get(I, 2) + IM + emission_of(e, 2),
      dp_get(I, 3) + IM + emission_of(e, 3),
      dp_get(I, 4) + IM + emission_of(e, 4),
      dp_get(I, 5) + IM + emission_of(e, 5),

      dp_get(D, 1) + DM + emission_of(e, 1),
      dp_get(D, 2) + DM + emission_of(e, 2),
      dp_get(D, 3) + DM + emission_of(e, 3),
      dp_get(D, 4) + DM + emission_of(e, 4),
      dp_get(D, 5) + DM + emission_of(e, 5),
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
      dp_get(M, 0) + MD + 0,
      dp_get(D, 0) + DD + 0,
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
  float x = dp_get(Mk, 1) + ME;
  // int src = MIX(0);
  int src = 0;
  for (int i = 2; i < 2 * core_size; i += 2)
  {
    Mk = dp_core_next(Mk);
    Dk = dp_core_next(Dk);
    // It is look_back=1 instead of look_back=0 because I already called
    // make_future(DPM) and make_future(DPD), for performance reasons.
    fmax_idx(&x, &src, dp_get(Mk, 1) + ME + 0, i + 0);
    fmax_idx(&x, &src, dp_get(Dk, 1) + DE + 0, i + 1);
  }
  trellis_set(t, STATE_E, src);
  return x;
}

INLINE float onto_J(struct trellis *t, float const E[restrict],
                    float const J[restrict], float const EJ, float const JJ,
                    float const e[restrict])
{
  // clang-format off
  float const x[] ALIGNED = {
      dp_get(E, 1) + EJ + emission_of(e, 1),
      dp_get(E, 2) + EJ + emission_of(e, 2),
      dp_get(E, 3) + EJ + emission_of(e, 3),
      dp_get(E, 4) + EJ + emission_of(e, 4),
      dp_get(E, 5) + EJ + emission_of(e, 5),

      dp_get(J, 1) + JJ + emission_of(e, 1),
      dp_get(J, 2) + JJ + emission_of(e, 2),
      dp_get(J, 3) + JJ + emission_of(e, 3),
      dp_get(J, 4) + JJ + emission_of(e, 4),
      dp_get(J, 5) + JJ + emission_of(e, 5),
  };
  if (!t) return reduce_fmax(array_size(x), x);
  // clang-format on

  int i = find_fmax(array_size(x), x);
  trellis_set(t, STATE_J, i);
  return x[i];
}

INLINE float onto_C(struct trellis *t, float const E[restrict],
                    float const C[restrict], float const EC, float const CC,
                    float const e[restrict])
{
  // clang-format off
  float const x[] ALIGNED = {
      dp_get(E, 1) + EC + emission_of(e, 1),
      dp_get(E, 2) + EC + emission_of(e, 2),
      dp_get(E, 3) + EC + emission_of(e, 3),
      dp_get(E, 4) + EC + emission_of(e, 4),
      dp_get(E, 5) + EC + emission_of(e, 5),

      dp_get(C, 1) + CC + emission_of(e, 1),
      dp_get(C, 2) + CC + emission_of(e, 2),
      dp_get(C, 3) + CC + emission_of(e, 3),
      dp_get(C, 4) + CC + emission_of(e, 4),
      dp_get(C, 5) + CC + emission_of(e, 5),
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
    dp_get(E, 0) + ET + 0,
    dp_get(C, 0) + CT + 0,
  };
  if (!t) return float_maximum(x[0], x[1]);
  // clang-format on

  int i = find_fmax(array_size(x), x);
  trellis_set(t, STATE_T, i);
  return x[i];
}

#endif
