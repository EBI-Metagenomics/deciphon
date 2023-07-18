#ifndef DECIPHON_HMM_READER_H
#define DECIPHON_HMM_READER_H

#include "entry_dist.h"
#include "hmmer_reader/hmmer_reader.h"
#include "imm/imm.h"
#include "model.h"
#include "model_params.h"
#include <stdio.h>

struct dcp_hmm_reader
{
  struct hmr hmr;
  struct hmr_prof protein;
  float null_lprobs[IMM_AMINO_SIZE];
  struct dcp_model model;
  bool end;
};

void dcp_hmm_reader_init(struct dcp_hmm_reader *, struct dcp_model_params,
                         FILE *);
int dcp_hmm_reader_next(struct dcp_hmm_reader *);
bool dcp_hmm_reader_end(struct dcp_hmm_reader const *);
void dcp_hmm_reader_cleanup(struct dcp_hmm_reader const *);

#endif
