#ifndef DECIPHON_VITERNI_H
#define DECIPHON_VITERNI_H

#include "viterbi_task.h"
#include <stdbool.h>
#include <stdio.h>

struct imm_eseq;
struct dcp_protein;

float dcp_viterbi_null(struct dcp_protein *, struct imm_eseq const *);
int dcp_viterbi(struct dcp_protein *, struct imm_eseq const *,
                struct dcp_viterbi_task *, bool const nopath);
void dcp_viterbi_dump(struct dcp_protein *, FILE *restrict);
void dcp_viterbi_dump_dot(struct dcp_protein *, FILE *restrict);

#endif
