#ifndef VITERBI_TASK_H
#define VITERBI_TASK_H

#include "imm/imm.h"
#include "trellis.h"
#include "trellis_bits.h"
#include "viterbi_dp.h"
#include <stdbool.h>

struct viterbi_task
{
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
  float score;
};

// clang-format off
void viterbi_task_init(struct viterbi_task *);
int  viterbi_task_setup(struct viterbi_task *, int core_size, int seq_size, bool const nopath);
void viterbi_task_cleanup(struct viterbi_task *);
// clang-format on

#endif
