#ifndef VITERBI_COREDP_H
#define VITERBI_COREDP_H

#include "compiler.h"
#include "state.h"
#include "xlimits.h"
#include <stddef.h>

#define DECLARE_COREDP(name) float *name

// clang-format off
void         coredp_init(float **x);
int          coredp_setup(float **, int core_size);
void         coredp_cleanup(float **);
CONST float *coredp_next(float *x) { return x + 3 * DCP_PAST_SIZE; }
// clang-format on

CONST float *coredp_rewind(float *x, int state)
{
  if (state == STATE_M) return x + 0 * DCP_PAST_SIZE;
  if (state == STATE_I) return x + 1 * DCP_PAST_SIZE;
  if (state == STATE_D) return x + 2 * DCP_PAST_SIZE;
  UNREACHABLE();
  return NULL;
}

#endif
