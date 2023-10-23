#ifndef VITERBI_DP_H
#define VITERBI_DP_H

#include "compiler.h"
#include "imm/imm.h"
#include "rc.h"
#include "state.h"
#include "trellis_bits.h"
#include <stdlib.h>

// clang-format off
int          dp_renew(float **, int core_size);
void         dp_del(float **);
CONST float *dp_next(float *x) { return x + 3 * VITERBI_PAST_SIZE; }
// clang-format on

CONST float *dp_rewind(float *x, int state)
{
  if (state == STATE_M) return x + 0 * VITERBI_PAST_SIZE;
  if (state == STATE_I) return x + 1 * VITERBI_PAST_SIZE;
  if (state == STATE_D) return x + 2 * VITERBI_PAST_SIZE;
  UNREACHABLE();
  return NULL;
}

#endif
