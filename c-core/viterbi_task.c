#include "viterbi_task.h"
#include "defer_return.h"
#include "rc.h"
#include "viterbi_dp.h"
#include <stdlib.h>

void viterbi_task_init(struct viterbi_task *x)
{
  x->dp = NULL;
  trellis_init(&x->trellis);
  x->path = imm_path();
  x->score = IMM_LPROB_NAN;
}

int viterbi_task_setup(struct viterbi_task *x, int core_size, int seq_size,
                       bool const nopath)
{
  int rc = 0;
  if ((rc = dp_renew(&x->dp, core_size))) defer_return(rc);

  for (int i = 0; i < VITERBI_PAST_SIZE; ++i)
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
    if (trellis_setup(&x->trellis, core_size, seq_size))
      defer_return(DCP_ENOMEM);

    imm_path_reset(&x->path);
  }

  return rc;

defer:
  viterbi_task_cleanup(x);
  return rc;
}

void viterbi_task_cleanup(struct viterbi_task *x)
{
  trellis_cleanup(&x->trellis);
  dp_del(&x->dp);
  imm_path_cleanup(&x->path);
}
