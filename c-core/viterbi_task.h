#ifndef VITERBI_TASK_H
#define VITERBI_TASK_H

#include "imm/imm.h"
#include "trellis.h"
#include "viterbi_dp.h"
#include <stdbool.h>

struct protein;

struct viterbi_task
{
  struct protein const *protein;
  float *dp;
  DECLARE_DP(S);
  DECLARE_DP(N);
  DECLARE_DP(B);
  DECLARE_DP(J);
  DECLARE_DP(E);
  DECLARE_DP(C);
  DECLARE_DP(T);
  struct trellis trellis;
  struct imm_path path;
};

// clang-format off
void viterbi_task_init(struct viterbi_task *);
int  viterbi_task_setup_protein(struct viterbi_task *, struct protein const *);
int  viterbi_task_setup_path(struct viterbi_task *, int seq_size);
void viterbi_task_cleanup(struct viterbi_task *);
// clang-format on

#endif
