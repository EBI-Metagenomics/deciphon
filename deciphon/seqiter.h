#ifndef DECIPHON_SEQITER_H
#define DECIPHON_SEQITER_H

#include "seq.h"
#include "seq_struct.h"

struct seqiter
{
  struct dcp_seq seq;
  dcp_seq_next_fn *next_callb;
  void *arg;
};

void dcp_seqiter_init(struct seqiter *);
void dcp_seqiter_set_callback(struct seqiter *, dcp_seq_next_fn *, void *);
bool dcp_seqiter_next(struct seqiter *);
struct dcp_seq const *dcp_seqiter_get(struct seqiter *);

#endif