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
  DECLARE_DP(S, VITERBI_PAST_SIZE);
  DECLARE_DP(N, VITERBI_PAST_SIZE);
  DECLARE_DP(B, VITERBI_PAST_SIZE);
  DECLARE_DP(J, VITERBI_PAST_SIZE);
  DECLARE_DP(E, VITERBI_PAST_SIZE);
  DECLARE_DP(C, VITERBI_PAST_SIZE);
  DECLARE_DP(T, VITERBI_PAST_SIZE);
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
