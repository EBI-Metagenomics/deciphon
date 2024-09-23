#include "batch.h"
#include "defer_return.h"
#include "error.h"
#include "queue.h"
#include "sequence.h"
#include <stdlib.h>

struct batch
{
  struct queue sequences;
};

struct batch *batch_new(void)
{
  struct batch *x = malloc(sizeof(struct batch));
  if (!x) return NULL;
  queue_init(&x->sequences);
  return x;
}

void batch_del(struct batch *x)
{
  if (x)
  {
    batch_reset(x);
    free(x);
  }
}

int batch_add(struct batch *x, long id, char const *name, char const *data)
{
  int rc = 0;
  struct sequence *seq = malloc(sizeof(*seq));

  if (!seq) defer_return(error(DCP_ENOMEM));
  if ((rc = sequence_init(seq, id, name, data))) defer_return(rc);

  queue_put(&x->sequences, &seq->node);
  return 0;

defer:
  free(seq);
  return rc;
}

void batch_reset(struct batch *x)
{
  struct sequence *seq = NULL;
  struct sequence *tmp = NULL;
  struct iter iter = queue_iter(&x->sequences);
  iter_for_each_entry_safe(seq, tmp, &iter, node)
  {
    sequence_cleanup(seq);
    free(seq);
  }
  queue_init(&x->sequences);
}

int batch_encode(struct batch *x, struct imm_code const *code)
{
  int rc = 0;
  struct sequence *seq = NULL;
  struct iter iter = queue_iter(&x->sequences);
  iter_for_each_entry(seq, &iter, node)
  {
    if ((rc = sequence_encode(seq, code))) break;
  }
  return rc;
}

struct iter batch_iter(struct batch const *x)
{
  return queue_iter(&x->sequences);
}
