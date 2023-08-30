#ifndef DECIPHON_NUCLT_DIST_H
#define DECIPHON_NUCLT_DIST_H

#include "imm/imm.h"

struct dcp_nuclt_dist
{
  struct imm_nuclt_lprob nucltp;
  struct imm_codon_marg codonm;
};

struct lip_file;

void dcp_nuclt_dist_init(struct dcp_nuclt_dist *, struct imm_nuclt const *);
int dcp_nuclt_dist_pack(struct dcp_nuclt_dist const *, struct lip_file *);
int dcp_nuclt_dist_unpack(struct dcp_nuclt_dist *, struct lip_file *);
void dcp_nuclt_dist_dump(struct dcp_nuclt_dist const *, FILE *restrict);

#endif
