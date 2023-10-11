#ifndef DECIPHON_VITERBI_TASK_H
#define DECIPHON_VITERBI_TASK_H

#include "compiler.h"
#include "imm/imm.h"
#include "trellis.h"
#include "viterbi_dp.h"
#include <stdbool.h>

struct dcp_viterbi_task
{
  float *dp;
  float S[DCP_VITERBI_PAST_SIZE];
  float N[DCP_VITERBI_PAST_SIZE];
  float B[DCP_VITERBI_PAST_SIZE];
  float J[DCP_VITERBI_PAST_SIZE];
  float E[DCP_VITERBI_PAST_SIZE];
  float C[DCP_VITERBI_PAST_SIZE];
  float T[DCP_VITERBI_PAST_SIZE];
  struct trellis trellis;
  struct imm_path path;
  float score;
};

void dcp_viterbi_task_init(struct dcp_viterbi_task *);
int dcp_viterbi_task_setup(struct dcp_viterbi_task *, int core_size,
                           int seq_size, bool const nopath);
void dcp_viterbi_task_cleanup(struct dcp_viterbi_task *);

DCP_CONST int lukbak(int i) { return i; }
DCP_CONST int nchars(int n) { return n - 1; }

#endif
