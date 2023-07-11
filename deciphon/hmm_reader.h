#ifndef DECIPHON_HMM_READER_H
#define DECIPHON_HMM_READER_H

#include "hmmer_reader/hmmer_reader.h"
#include "imm/imm.h"
#include "model.h"
#include "rc.h"
#include <stdio.h>

struct dcp_hmm_reader
{
  struct hmr hmr;
  struct hmr_prof protein;
  float null_lprobs[IMM_AMINO_SIZE];
  struct dcp_model model;
  bool end;
};

void dcp_hmm_reader_init(struct dcp_hmm_reader *, struct imm_gencode const *,
                         struct imm_amino const *,
                         struct imm_nuclt_code const *, enum dcp_entry_dist,
                         float epsilon, FILE *);
int dcp_hmm_reader_next(struct dcp_hmm_reader *);
bool dcp_hmm_reader_end(struct dcp_hmm_reader const *);
void dcp_hmm_reader_del(struct dcp_hmm_reader const *);

#endif
