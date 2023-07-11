#ifndef DECIPHON_SEQ_ITER_H
#define DECIPHON_SEQ_ITER_H

#include "seq.h"
#include "seq_struct.h"

struct dcp_seq_iter
{
  struct dcp_seq seq;
  dcp_seq_next_fn *next_callb;
  void *arg;
};

void dcp_seq_iter_init(struct dcp_seq_iter *);
void dcp_seq_iter_set_callback(struct dcp_seq_iter *, dcp_seq_next_fn *,
                               void *);
bool dcp_seq_iter_next(struct dcp_seq_iter *);
struct dcp_seq const *dcp_seq_iter_get(struct dcp_seq_iter *);

#endif
