#include "viterbi_task.h"
#include "defer_return.h"
#include "rc.h"
#include <stdlib.h>

void dcp_viterbi_task_init(struct dcp_viterbi_task *x)
{
  x->dp = NULL;
  imm_trellis_init(&x->trellis);
  x->path = imm_path();
  x->score = IMM_LPROB_NAN;
}

int dcp_viterbi_task_setup(struct dcp_viterbi_task *x, int core_size,
                           int seq_size, bool const nopath)
{
  int rc = 0;
  if ((rc = dp_renew(&x->dp, core_size))) defer_return(rc);

  for (int i = 0; i < DCP_VITERBI_PAST_SIZE; ++i)
  {
    x->S[i] = IMM_LPROB_ZERO;
    x->N[i] = IMM_LPROB_ZERO;
    x->B[i] = IMM_LPROB_ZERO;
    x->J[i] = IMM_LPROB_ZERO;
    x->E[i] = IMM_LPROB_ZERO;
    x->C[i] = IMM_LPROB_ZERO;
    x->T[i] = IMM_LPROB_ZERO;
  }

  if (!nopath)
  {
    int nstates = 3 + 3 * core_size + 3 + 1;
    if (imm_trellis_setup(&x->trellis, seq_size, nstates))
      defer_return(DCP_ENOMEM);
    imm_trellis_prepare(&x->trellis);

    imm_path_reset(&x->path);
  }

  return rc;

defer:
  dcp_viterbi_task_cleanup(x);
  return rc;
}

void dcp_viterbi_task_cleanup(struct dcp_viterbi_task *x)
{
  imm_trellis_cleanup(&x->trellis);
  dp_del(&x->dp);
  imm_path_cleanup(&x->path);
}
