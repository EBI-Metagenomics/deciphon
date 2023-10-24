#include "viterbi_dp.h"
#include "imm/imm.h"
#include "xrealloc.h"

int dp_core_renew(float **dp, int core_size)
{
  size_t size = 3 * PAST_SIZE * core_size;

  *dp = xrealloc(*dp, sizeof(float) * size);
  if (!*dp && size > 0) return DCP_ENOMEM;

  for (size_t i = 0; i < size; ++i)
    (*dp)[i] = IMM_LPROB_ZERO;

  return 0;
}

void dp_core_del(float **dp)
{
  free(*dp);
  *dp = NULL;
}
