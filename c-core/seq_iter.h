#ifndef SEQ_ITER_H
#define SEQ_ITER_H

#include "seq.h"
#include "seq_struct.h"

struct seq_iter
{
  struct seq seq;
  seq_next_fn *next_callb;
  void *arg;
};

void seq_iter_init(struct seq_iter *, struct imm_code const *, seq_next_fn *,
                   void *arg);
void seq_iter_cleanup(struct seq_iter *);
bool seq_iter_next(struct seq_iter *);
struct seq *seq_iter_get(struct seq_iter *);

#endif
