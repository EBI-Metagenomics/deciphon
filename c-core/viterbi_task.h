#ifndef VITERBI_TASK_H
#define VITERBI_TASK_H

#include "compiler.h"
#include "imm/imm.h"
#include "trellis.h"
#include "trellis_bits.h"
#include <stdbool.h>

struct viterbi_task
{
  float *dp;
  float S[VITERBI_PAST_SIZE];
  float N[VITERBI_PAST_SIZE];
  float B[VITERBI_PAST_SIZE];
  float J[VITERBI_PAST_SIZE];
  float E[VITERBI_PAST_SIZE];
  float C[VITERBI_PAST_SIZE];
  float T[VITERBI_PAST_SIZE];
  struct trellis trellis;
  struct imm_path path;
  float score;
};

// clang-format off
void viterbi_task_init(struct viterbi_task *);
int  viterbi_task_setup(struct viterbi_task *, int core_size, int seq_size,
                        bool const nopath);
void viterbi_task_cleanup(struct viterbi_task *);
// clang-format on

CONST int lukbak(int i) { return i; }
CONST int nchars(int n) { return n - 1; }

#endif
