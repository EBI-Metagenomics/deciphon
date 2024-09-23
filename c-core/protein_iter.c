#include "protein_iter.h"
#include "protein.h"
#include "protein_reader.h"
#include <string.h>

void protein_iter_init(struct protein_iter *x)
{
  x->start_idx = -1;
  x->curr_idx  = -1;
  x->end_idx   = -1;
  x->offset    = -1;
  memset(&x->file, 0, sizeof(x->file));
  x->reader    = NULL;
}

void protein_iter_setup(struct protein_iter *x, struct protein_reader *reader,
                        int start_idx, int end_idx, long offset, int fd)
{
  x->start_idx = start_idx;
  x->curr_idx  = start_idx - 1;
  x->end_idx   = end_idx;
  x->offset    = offset;
  lio_setup(&x->file, fd);
  x->reader    = reader;
}

int protein_iter_rewind(struct protein_iter *x)
{
  x->curr_idx = x->start_idx - 1;
  return lio_seek(&x->file, x->offset);
}

int protein_iter_next(struct protein_iter *x, struct protein *protein)
{
  x->curr_idx += 1;
  if (protein_iter_end(x)) return 0;
  return protein_unpack(protein, &x->file);
}

bool protein_iter_end(struct protein_iter const *x)
{
  return x->curr_idx == x->end_idx;
}

int protein_iter_idx(struct protein_iter const *x) { return x->curr_idx; }
