#ifndef SEQ_ITER_H
#define SEQ_ITER_H

#include "seq.h"
#include "seq_struct.h"

struct dcp_seq_iter
{
  struct dcp_seq seq;
  dcp_seq_next_fn *next_callb;
  void *arg;
};

void dcp_seq_iter_init(struct dcp_seq_iter *, struct imm_code const *,
                       dcp_seq_next_fn *callb, void *arg);
void dcp_seq_iter_cleanup(struct dcp_seq_iter *);
bool dcp_seq_iter_next(struct dcp_seq_iter *);
struct dcp_seq *dcp_seq_iter_get(struct dcp_seq_iter *);

#endif
