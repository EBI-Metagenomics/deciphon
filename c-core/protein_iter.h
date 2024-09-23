#ifndef PROTEIN_ITER_H
#define PROTEIN_ITER_H

#include "lio.h"
#include "protein.h"
#include <stdbool.h>

struct protein_reader;

struct protein_iter
{
  int start_idx;
  int curr_idx;
  int end_idx;
  long offset;
  struct lio_reader file;
  struct protein_reader *reader;
};

void protein_iter_init(struct protein_iter *);
void protein_iter_setup(struct protein_iter *, struct protein_reader *, int start_idx, int end_idx, long offset, int);
int  protein_iter_rewind(struct protein_iter *);
int  protein_iter_next(struct protein_iter *, struct protein *);
bool protein_iter_end(struct protein_iter const *);
int  protein_iter_idx(struct protein_iter const *);

#endif
