#ifndef DECIPHON_VITERBI_TASK_H
#define DECIPHON_VITERBI_TASK_H

#include "imm/imm.h"
#include <stdbool.h>

#define DCP_VITERBI_PAST_SIZE 6

struct dcp_viterbi_task
{
  float *dp;
  struct imm_trellis trellis;
  struct imm_path path;
  float score;
};

void dcp_viterbi_task_init(struct dcp_viterbi_task *);
int dcp_viterbi_task_setup(struct dcp_viterbi_task *, int core_size,
                           int seq_size);
void dcp_viterbi_task_cleanup(struct dcp_viterbi_task *);

#endif
