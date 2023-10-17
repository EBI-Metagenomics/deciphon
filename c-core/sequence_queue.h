#ifndef SEQUENCE_QUEUE_H
#define SEQUENCE_QUEUE_H

#include "compiler.h"
#include "queue.h"

struct imm_code;

struct sequence_queue
{
  struct imm_code const *code;
  struct queue sequences;
};

void sequence_queue_init(struct sequence_queue *, struct imm_code const *);
int sequence_queue_add(struct sequence_queue *, long id, char const *name,
                       char const *data);
void sequence_queue_cleanup(struct sequence_queue *);

#endif
