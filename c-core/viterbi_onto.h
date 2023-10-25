#ifndef VITERBI_ONTO_H
#define VITERBI_ONTO_H

#include "array_size.h"
#include "compiler.h"
#include "find_fmax.h"
#include "reduce_fmax.h"
#include "trellis.h"
#include "viterbi_coredp.h"
#include "viterbi_dp.h"
#include "viterbi_table.h"

PURE float onto_R(float const S[restrict], float const R[restrict],
                  float const RR, float const e[restrict])
{
  // clang-format off
  float const x[] ALIGNED = {
      dp_get(S, 1) + 0 + table_get(e, 1),
      dp_get(S, 2) + 0 + table_get(e, 2),
      dp_get(S, 3) + 0 + table_get(e, 3),
      dp_get(S, 4) + 0 + table_get(e, 4),
      dp_get(S, 5) + 0 + table_get(e, 5),

      dp_get(R, 1) + RR + table_get(e, 1),
      dp_get(R, 2) + RR + table_get(e, 2),
      dp_get(R, 3) + RR + table_get(e, 3),
      dp_get(R, 4) + RR + table_get(e, 4),
      dp_get(R, 5) + RR + table_get(e, 5),
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
      dp_get(S, 1) + SN + table_get(e, 1),
      dp_get(S, 2) + SN + table_get(e, 2),
      dp_get(S, 3) + SN + table_get(e, 3),
      dp_get(S, 4) + SN + table_get(e, 4),
      dp_get(S, 5) + SN + table_get(e, 5),

      dp_get(N, 1) + NN + table_get(e, 1),
      dp_get(N, 2) + NN + table_get(e, 2),
      dp_get(N, 3) + NN + table_get(e, 3),
      dp_get(N, 4) + NN + table_get(e, 4),
      dp_get(N, 5) + NN + table_get(e, 5),
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
      dp_get(B, 1) + BM + table_get(e, 1),
      dp_get(B, 2) + BM + table_get(e, 2),
      dp_get(B, 3) + BM + table_get(e, 3),
      dp_get(B, 4) + BM + table_get(e, 4),
      dp_get(B, 5) + BM + table_get(e, 5),
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
      dp_get(M, 1) + MI + table_get(e, 1),
      dp_get(M, 2) + MI + table_get(e, 2),
      dp_get(M, 3) + MI + table_get(e, 3),
      dp_get(M, 4) + MI + table_get(e, 4),
      dp_get(M, 5) + MI + table_get(e, 5),

      dp_get(I, 1) + II + table_get(e, 1),
      dp_get(I, 2) + II + table_get(e, 2),
      dp_get(I, 3) + II + table_get(e, 3),
      dp_get(I, 4) + II + table_get(e, 4),
      dp_get(I, 5) + II + table_get(e, 5),
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
      dp_get(B, 1) + BM + table_get(e, 1),
      dp_get(B, 2) + BM + table_get(e, 2),
      dp_get(B, 3) + BM + table_get(e, 3),
      dp_get(B, 4) + BM + table_get(e, 4),
      dp_get(B, 5) + BM + table_get(e, 5),

      dp_get(M, 1) + MM + table_get(e, 1),
      dp_get(M, 2) + MM + table_get(e, 2),
      dp_get(M, 3) + MM + table_get(e, 3),
      dp_get(M, 4) + MM + table_get(e, 4),
      dp_get(M, 5) + MM + table_get(e, 5),

      dp_get(I, 1) + IM + table_get(e, 1),
      dp_get(I, 2) + IM + table_get(e, 2),
      dp_get(I, 3) + IM + table_get(e, 3),
      dp_get(I, 4) + IM + table_get(e, 4),
      dp_get(I, 5) + IM + table_get(e, 5),

      dp_get(D, 1) + DM + table_get(e, 1),
      dp_get(D, 2) + DM + table_get(e, 2),
      dp_get(D, 3) + DM + table_get(e, 3),
      dp_get(D, 4) + DM + table_get(e, 4),
      dp_get(D, 5) + DM + table_get(e, 5),
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
  float *Mk = coredp_rewind(dp, STATE_M);
  float *Dk = coredp_rewind(dp, STATE_D);
  float x = dp_get(Mk, 1) + ME;
  // int src = MIX(0);
  int src = 0;
  for (int i = 2; i < 2 * core_size; i += 2)
  {
    Mk = coredp_next(Mk);
    Dk = coredp_next(Dk);
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
      dp_get(E, 1) + EJ + table_get(e, 1),
      dp_get(E, 2) + EJ + table_get(e, 2),
      dp_get(E, 3) + EJ + table_get(e, 3),
      dp_get(E, 4) + EJ + table_get(e, 4),
      dp_get(E, 5) + EJ + table_get(e, 5),

      dp_get(J, 1) + JJ + table_get(e, 1),
      dp_get(J, 2) + JJ + table_get(e, 2),
      dp_get(J, 3) + JJ + table_get(e, 3),
      dp_get(J, 4) + JJ + table_get(e, 4),
      dp_get(J, 5) + JJ + table_get(e, 5),
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
      dp_get(E, 1) + EC + table_get(e, 1),
      dp_get(E, 2) + EC + table_get(e, 2),
      dp_get(E, 3) + EC + table_get(e, 3),
      dp_get(E, 4) + EC + table_get(e, 4),
      dp_get(E, 5) + EC + table_get(e, 5),

      dp_get(C, 1) + CC + table_get(e, 1),
      dp_get(C, 2) + CC + table_get(e, 2),
      dp_get(C, 3) + CC + table_get(e, 3),
      dp_get(C, 4) + CC + table_get(e, 4),
      dp_get(C, 5) + CC + table_get(e, 5),
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
