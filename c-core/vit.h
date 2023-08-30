#ifndef DECIPHON_VIT_H
#define DECIPHON_VIT_H

#include <stdio.h>

struct dcp_scan_thrd;
struct imm_eseq;
struct p7;

float vit_null(struct p7 *, struct imm_eseq const *);
float vit(struct p7 *, struct imm_eseq const *);
void vit_dump(struct p7 *, FILE *restrict);
void vit_dump_dot(struct p7 *, FILE *restrict);

#endif
