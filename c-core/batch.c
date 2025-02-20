#include "batch.h"
#include "deciphon.h"
#include "defer_return.h"
#include "error.h"
#include "queue.h"
#include "sequence.h"
#include <stdlib.h>

struct dcp_batch
{
  struct queue sequences;
};

struct dcp_batch *dcp_batch_new(void)
{
  struct dcp_batch *x = malloc(sizeof(struct dcp_batch));
  if (!x) return NULL;
  queue_init(&x->sequences);
  return x;
}

void dcp_batch_del(struct dcp_batch *x)
{
  if (x)
  {
    dcp_batch_reset(x);
    free(x);
  }
}

int dcp_batch_add(struct dcp_batch *x, long id, char const *name, char const *data)
{
  int rc = 0;
  struct sequence *seq = malloc(sizeof(*seq));

  if (!seq) defer_return(error(DCP_ENOMEM));
  if ((rc = sequence_init(seq, id, name, data))) defer_return(error(rc));

  queue_put(&x->sequences, &seq->node);
  return 0;

defer:
  free(seq);
  return rc;
}

void dcp_batch_reset(struct dcp_batch *x)
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

int batch_encode(struct dcp_batch const *x, struct imm_code const *code)
{
  int rc = 0;
  struct sequence *seq = NULL;
  struct iter iter = queue_iter(&x->sequences);
  iter_for_each_entry(seq, &iter, node)
  {
    if ((rc = sequence_encode(seq, code))) return error(rc);
  }
  return 0;
}

struct iter batch_iter(struct dcp_batch const *x)
{
  return queue_iter(&x->sequences);
}
