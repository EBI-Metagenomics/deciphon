#ifndef NUCLT_DIST_H
#define NUCLT_DIST_H

#include "imm/codon_marg.h"
#include "imm/nuclt_lprob.h"
#include <stdio.h>

struct nuclt_dist
{
  struct imm_nuclt_lprob nucltp;
  struct imm_codon_marg codonm;
};

struct lip_file;
struct imm_nuclt;

// clang-format off
void nuclt_dist_init(struct nuclt_dist *, struct imm_nuclt const *);
int  nuclt_dist_pack(struct nuclt_dist const *, struct lip_file *);
int  nuclt_dist_unpack(struct nuclt_dist *, struct lip_file *);
void nuclt_dist_dump(struct nuclt_dist const *, FILE *restrict);
// clang-format on

#endif
