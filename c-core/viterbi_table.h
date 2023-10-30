#ifndef VITERBI_TABLE_H
#define VITERBI_TABLE_H

#include "compiler.h"
#include "xlimits.h"
#include <math.h>
#include <stdbool.h>

#define DECLARE_TABLE(name) float name[DCP_PAST_SIZE - 1] ALIGNED

INLINE void table_setup(float x[restrict], float const emission[restrict],
                        int const index[restrict], bool const safe)
{
#pragma GCC unroll(DCP_PAST_SIZE - 1)
  for (int i = 0; i < DCP_PAST_SIZE - 1; ++i)
    x[i] = (!safe && index[i] < 0) ? -INFINITY : emission[index[i]];
}

INLINE void table_prefetch(float const emission[restrict],
                           int const index[restrict])
{
#pragma GCC unroll(DCP_PAST_SIZE - 1)
  for (int i = 0; i < DCP_PAST_SIZE - 1; ++i)
    PREFETCH(emission + index[i], 0, 1);
}

CONST float table_get(float const emission[restrict], int num_chars)
{
  return emission[num_chars - 1];
}

#endif
