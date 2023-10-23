#include "viterbi_dp.h"
#include "xrealloc.h"

int dp_renew(float **dp, int core_size)
{
  size_t size = 3 * VITERBI_PAST_SIZE * core_size;

  *dp = xrealloc(*dp, sizeof(float) * size);
  if (!*dp && size > 0) return DCP_ENOMEM;

  for (size_t i = 0; i < size; ++i)
    (*dp)[i] = IMM_LPROB_ZERO;

  return 0;
}

void dp_del(float **dp)
{
  free(*dp);
  *dp = NULL;
}
