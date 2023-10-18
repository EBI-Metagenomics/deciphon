#include "sequence_queue.h"
#include "defer_return.h"
#include "queue.h"
#include "rc.h"
#include "sequence.h"
#include <stdlib.h>

void sequence_queue_init(struct sequence_queue *x)
{
  x->code = NULL;
  queue_init(&x->sequences);
}

void sequence_queue_setup(struct sequence_queue *x, struct imm_code const *code)
{
  x->code = code;
}

int sequence_queue_put(struct sequence_queue *x, long id, char const *name,
                       char const *data)
{
  int rc = 0;
  struct sequence *seq = malloc(sizeof(*seq));

  if (!seq) defer_return(DCP_ENOMEM);
  if ((rc = sequence_init(seq, x->code, id, name, data))) defer_return(rc);

  queue_put(&x->sequences, &seq->node);
  return 0;

defer:
  free(seq);
  return rc;
}

struct iter sequence_queue_iter(struct sequence_queue const *x)
{
  return queue_iter(&x->sequences);
}

void sequence_queue_cleanup(struct sequence_queue *x)
{
  struct iter iter = queue_iter(&x->sequences);
  struct sequence *tmp = NULL;
  struct sequence *seq = NULL;
  iter_for_each_entry_safe(seq, tmp, &iter, node)
  {
    sequence_cleanup(seq);
    free(seq);
  }
  queue_init(&x->sequences);
}
