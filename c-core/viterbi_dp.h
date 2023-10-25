#ifndef VITERBI_DP_H
#define VITERBI_DP_H

#include "compiler.h"
#include "xlimits.h"
#include <string.h>

#define DECLARE_DP(name) float name[DCP_PAST_SIZE] ALIGNED

INLINE void dp_fill(float *x, float value)
{
  for (int i = 0; i < DCP_PAST_SIZE; ++i)
    x[i] = value;
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
  memmove(&x[1], &x[0], sizeof(float) * (DCP_PAST_SIZE - 1));
}

#endif
