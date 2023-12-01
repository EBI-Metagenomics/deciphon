#ifndef HMM_READER_H
#define HMM_READER_H

#include "hmmer_reader/hmmer_reader.h"
#include "model.h"
#include "model_params.h"
#include <stdio.h>

struct hmm_reader
{
  struct hmr hmr;
  struct hmr_prof protein;
  float null_lprobs[IMM_AMINO_SIZE];
  struct model model;
  bool end;
};

// clang-format off
int  hmm_reader_init(struct hmm_reader *, struct model_params, FILE *);
int  hmm_reader_next(struct hmm_reader *);
bool hmm_reader_end(struct hmm_reader const *);
void hmm_reader_cleanup(struct hmm_reader *);
// clang-format on

#endif
