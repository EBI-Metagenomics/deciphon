#ifndef SEQUENCE_QUEUE_H
#define SEQUENCE_QUEUE_H

#include "queue.h"

struct imm_code;

struct sequence_queue
{
  struct imm_code const *code;
  struct queue sequences;
};

// clang-format off
void sequence_queue_init(struct sequence_queue *, struct imm_code const *);
int  sequence_queue_put(struct sequence_queue *, long id, char const *name, char const *data);
void sequence_queue_cleanup(struct sequence_queue *);
// clang-format on

#endif