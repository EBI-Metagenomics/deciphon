#ifndef DECIPHON_VIT_H
#define DECIPHON_VIT_H

#include "imm/imm.h"
#include "viterbi_task.h"
#include <stdbool.h>
#include <stdio.h>

struct dcp_scan_thrd;
struct imm_eseq;
struct imm_prod;
struct p7;

float dcp_vit_null(struct p7 *, struct imm_eseq const *);
int dcp_vit(struct p7 *, struct imm_eseq const *, struct dcp_viterbi_task *);
void dcp_vit_dump(struct p7 *, FILE *restrict);
void dcp_vit_dump_dot(struct p7 *, FILE *restrict);

#endif
