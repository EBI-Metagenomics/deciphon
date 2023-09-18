#ifndef DECIPHON_PROTEIN_ITER_H
#define DECIPHON_PROTEIN_ITER_H

#include "lip/lip.h"
#include "p7.h"
#include <stdbool.h>

struct dcp_protein_reader;

struct dcp_protein_iter
{
  int partition;
  int start_idx;
  int curr_idx;
  long offset;
  FILE *fp;
  struct lip_file file;
  struct dcp_protein_reader *reader;
};

void dcp_protein_iter_init(struct dcp_protein_iter *,
                           struct dcp_protein_reader *, int partition,
                           int start_idx, long offset, FILE *);
int dcp_protein_iter_rewind(struct dcp_protein_iter *);
int dcp_protein_iter_next(struct dcp_protein_iter *, struct p7 *);
bool dcp_protein_iter_end(struct dcp_protein_iter const *);
int dcp_protein_iter_idx(struct dcp_protein_iter const *);

#endif
