#include "seq_iter.h"
#include "seq.h"

static bool noop(struct dcp_seq *x, void *arg)
{
  (void)x;
  (void)arg;
  return false;
}

void dcp_seq_iter_init(struct dcp_seq_iter *x, struct imm_code const *code)
{
  dcp_seq_init(&x->seq, code);
  x->next_callb = noop;
  x->arg = NULL;
}

void dcp_seq_iter_cleanup(struct dcp_seq_iter *x) { dcp_seq_cleanup(&x->seq); }

void dcp_seq_iter_set_callback(struct dcp_seq_iter *x, dcp_seq_next_fn *callb,
                               void *arg)
{
  x->next_callb = callb ? callb : noop;
  x->arg = arg;
}

bool dcp_seq_iter_next(struct dcp_seq_iter *x)
{
  return (*x->next_callb)(&x->seq, x->arg);
}

struct dcp_seq *dcp_seq_iter_get(struct dcp_seq_iter *x) { return &x->seq; }
