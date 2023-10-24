#include "viterbi_task.h"
#include "defer_return.h"
#include "rc.h"
#include <stdlib.h>

void viterbi_task_init(struct viterbi_task *x)
{
  x->core_size = 0;
  dp_core_init(&x->dp);
  trellis_init(&x->trellis);
  x->path = imm_path();
  x->score = IMM_LPROB_NAN;
}

int viterbi_task_setup(struct viterbi_task *x, int core_size)
{
  x->core_size = core_size;

  dp_fill(x->S, IMM_LPROB_ZERO);
  dp_fill(x->N, IMM_LPROB_ZERO);
  dp_fill(x->B, IMM_LPROB_ZERO);
  dp_fill(x->J, IMM_LPROB_ZERO);
  dp_fill(x->E, IMM_LPROB_ZERO);
  dp_fill(x->C, IMM_LPROB_ZERO);
  dp_fill(x->T, IMM_LPROB_ZERO);

  return dp_core_renew(&x->dp, core_size);
}

int viterbi_task_setup_path(struct viterbi_task *x, int seq_size)
{
  imm_path_reset(&x->path);
  return trellis_setup(&x->trellis, x->core_size, seq_size);
}

void viterbi_task_cleanup(struct viterbi_task *x)
{
  trellis_cleanup(&x->trellis);
  dp_core_del(&x->dp);
  imm_path_cleanup(&x->path);
}
