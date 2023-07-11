#ifndef DECIPHON_PROTEINITER_H
#define DECIPHON_PROTEINITER_H

#include "lip/lip.h"
#include "protein.h"
#include <stdbool.h>

struct dcp_protein_reader;

struct dcp_proteiniter
{
  int partition;
  int start_idx;
  int curr_idx;
  long offset;
  FILE *fp;
  struct lip_file file;
  struct dcp_protein_reader *reader;
};

void dcp_proteiniter_init(struct dcp_proteiniter *, struct dcp_protein_reader *,
                          int partition, int start_idx, long offset, FILE *);
int dcp_proteiniter_rewind(struct dcp_proteiniter *);
int dcp_proteiniter_next(struct dcp_proteiniter *, struct dcp_protein *);
bool dcp_proteiniter_end(struct dcp_proteiniter const *);
int dcp_protein_iter_idx(struct dcp_proteiniter const *);

#endif
