#include "seqiter.h"
#include "seq.h"

static bool noop(struct dcp_seq *x, void *arg)
{
  (void)x;
  (void)arg;
  return false;
}

void dcp_seqiter_init(struct seqiter *x)
{
  dcp_seq_init(&x->seq);
  x->next_callb = noop;
  x->arg = NULL;
}

void dcp_seqiter_set_callback(struct seqiter *x, dcp_seq_next_fn *callb,
                              void *arg)
{
  x->next_callb = callb ? callb : noop;
  x->arg = arg;
}

bool dcp_seqiter_next(struct seqiter *x)
{
  return (*x->next_callb)(&x->seq, x->arg);
}

struct dcp_seq const *dcp_seqiter_get(struct seqiter *x) { return &x->seq; }
