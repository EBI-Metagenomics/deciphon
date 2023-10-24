#ifndef VITERBI_DP_H
#define VITERBI_DP_H

#include "compiler.h"
#include "imm/imm.h"
#include "rc.h"
#include "state.h"
#include "trellis_bits.h"
#include <stdlib.h>
#include <string.h>

// clang-format off
int          dp_core_renew(float **, int core_size);
void         dp_core_del(float **);
CONST float *dp_core_next(float *x) { return x + 3 * PAST_SIZE; }
// clang-format on

#define DECLARE_DP(name) float name[PAST_SIZE] ALIGNED

INLINE void dp_fill(float *x, float value)
{
  for (int i = 0; i < PAST_SIZE; ++i)
    x[i] = value;
}

CONST float *dp_rewind(float *x, int state)
{
  if (state == STATE_M) return x + 0 * PAST_SIZE;
  if (state == STATE_I) return x + 1 * PAST_SIZE;
  if (state == STATE_D) return x + 2 * PAST_SIZE;
  UNREACHABLE();
  return NULL;
}

CONST float dp_get(float const x[restrict], int look_back)
{
  return x[look_back];
}

INLINE void dp_set(float x[restrict], int look_back, float value)
{
  x[look_back] = value;
}

INLINE void dp_advance(float x[])
{
  memmove(&x[1], &x[0], sizeof(float) * (PAST_SIZE - 1));
}

#endif
