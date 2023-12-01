#ifndef PROTEIN_ITER_H
#define PROTEIN_ITER_H

#include "lip/file/file.h"
#include "protein.h"
#include <stdbool.h>

struct protein_reader;

struct protein_iter
{
  int start_idx;
  int curr_idx;
  int end_idx;
  long offset;
  FILE *fp;
  struct lip_file file;
  struct protein_reader *reader;
};

// clang-format off
void protein_iter_init(struct protein_iter *, struct protein_reader *, int start_idx, int end_idx, long offset, FILE *);
int  protein_iter_rewind(struct protein_iter *);
int  protein_iter_next(struct protein_iter *, struct protein *);
bool protein_iter_end(struct protein_iter const *);
int  protein_iter_idx(struct protein_iter const *);
// clang-format on

#endif
