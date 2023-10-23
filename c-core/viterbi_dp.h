#ifndef VITERBI_DP_H
#define VITERBI_DP_H

#include "compiler.h"
#include "imm/imm.h"
#include "rc.h"
#include "state.h"
#include <stdlib.h>

enum
{
  DCP_VITERBI_PAST_SIZE = 6
};

INLINE int dp_renew(float **dp, int core_size)
{
  int size = 3 * DCP_VITERBI_PAST_SIZE * core_size;
  void *ptr = realloc(*dp, sizeof(float) * size);
  if (!ptr && size > 0)
  {
    free(*dp);
    *dp = NULL;
    return DCP_ENOMEM;
  }
  *dp = ptr;
  for (int i = 0; i < size; ++i)
    (*dp)[i] = IMM_LPROB_ZERO;
  return 0;
}

INLINE void dp_del(float **dp)
{
  free(*dp);
  *dp = NULL;
}

CONST float *dp_rewind(float *x, int state)
{
  if (state == STATE_M) return x + 0 * DCP_VITERBI_PAST_SIZE;
  if (state == STATE_I) return x + 1 * DCP_VITERBI_PAST_SIZE;
  if (state == STATE_D) return x + 2 * DCP_VITERBI_PAST_SIZE;
  UNREACHABLE();
  return NULL;
}

CONST float *dp_next(float *x) { return x + 3 * DCP_VITERBI_PAST_SIZE; }

#endif
