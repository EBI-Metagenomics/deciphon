#include "viterbi_task.h"
#include "defer_return.h"
#include "protein.h"
#include "rc.h"
#include "viterbi_coredp.h"
#include <stdlib.h>

void viterbi_task_init(struct viterbi_task *x)
{
  x->protein = NULL;
  coredp_init(&x->dp);
  trellis_init(&x->trellis);
  x->path = imm_path();
}

int viterbi_task_setup_protein(struct viterbi_task *x,
                               struct protein const *protein)
{
  x->protein = protein;

  dp_fill(x->S, IMM_LPROB_ZERO);
  dp_fill(x->N, IMM_LPROB_ZERO);
  dp_fill(x->B, IMM_LPROB_ZERO);
  dp_fill(x->J, IMM_LPROB_ZERO);
  dp_fill(x->E, IMM_LPROB_ZERO);
  dp_fill(x->C, IMM_LPROB_ZERO);
  dp_fill(x->T, IMM_LPROB_ZERO);

  return coredp_setup(&x->dp, x->protein->core_size);
}

int viterbi_task_setup_path(struct viterbi_task *x, int seq_size)
{
  imm_path_reset(&x->path);
  return trellis_setup(&x->trellis, x->protein->core_size, seq_size);
}

void viterbi_task_cleanup(struct viterbi_task *x)
{
  trellis_cleanup(&x->trellis);
  coredp_cleanup(&x->dp);
  imm_path_cleanup(&x->path);
}
