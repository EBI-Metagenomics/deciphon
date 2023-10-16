#include "seq_iter.h"
#include "seq.h"

static bool noop(struct seq *x, void *arg)
{
  (void)x;
  (void)arg;
  return false;
}

void seq_iter_init(struct seq_iter *x, struct imm_code const *code,
                       seq_next_fn *callb, void *arg)
{
  seq_init(&x->seq, code);
  x->next_callb = callb ? callb : noop;
  x->arg = arg;
}

void seq_iter_cleanup(struct seq_iter *x) { seq_cleanup(&x->seq); }

void dcp_seq_iter_set_callback(struct seq_iter *x, seq_next_fn *callb,
                               void *arg)
{
  x->next_callb = callb ? callb : noop;
  x->arg = arg;
}

bool seq_iter_next(struct seq_iter *x)
{
  return (*x->next_callb)(&x->seq, x->arg);
}

struct seq *seq_iter_get(struct seq_iter *x) { return &x->seq; }
