#ifndef NUCLT_DIST_H
#define NUCLT_DIST_H

#include "imm_codon_marg.h"
#include "imm_nuclt_lprob.h"
#include <stdio.h>

struct nuclt_dist
{
  struct imm_nuclt_lprob nucltp;
  struct imm_codon_marg codonm;
};

struct lio_writer;
struct lio_reader;
struct imm_nuclt;

void nuclt_dist_init(struct nuclt_dist *, struct imm_nuclt const *);
int  nuclt_dist_pack(struct nuclt_dist const *, struct lio_writer *);
int  nuclt_dist_unpack(struct nuclt_dist *, struct lio_reader *);
void nuclt_dist_dump(struct nuclt_dist const *, FILE *restrict);

#endif
