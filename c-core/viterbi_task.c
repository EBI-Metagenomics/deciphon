#include "viterbi_task.h"
#include "defer_return.h"
#include "rc.h"
#include <stdlib.h>

void dcp_viterbi_task_init(struct dcp_viterbi_task *x)
{
  x->dp = NULL;
  imm_trellis_init(&x->trellis);
  x->path = imm_path();
}

int dcp_viterbi_task_setup(struct dcp_viterbi_task *x, int core_size,
                           int seq_size)
{
  int rc = 0;
  int dp_size = 3 * DCP_VITERBI_PAST_SIZE * core_size;
  void *ptr = realloc(x->dp, sizeof(*x->dp) * dp_size);
  if (!ptr && dp_size > 0) defer_return(DCP_ENOMEM);
  x->dp = ptr;

  for (int i = 0; i < dp_size; ++i)
    x->dp[i] = IMM_LPROB_ZERO;

  int nstates = 3 + 3 * core_size + 3 + 1;
  if (imm_trellis_setup(&x->trellis, seq_size, nstates))
    defer_return(DCP_ENOMEM);
  imm_trellis_prepare(&x->trellis);

  imm_path_reset(&x->path);

  return rc;

defer:
  dcp_viterbi_task_cleanup(x);
  return rc;
}

void dcp_viterbi_task_cleanup(struct dcp_viterbi_task *x)
{
  imm_trellis_cleanup(&x->trellis);
  if (x->dp)
  {
    free(x->dp);
    x->dp = NULL;
  }
  imm_path_cleanup(&x->path);
}
