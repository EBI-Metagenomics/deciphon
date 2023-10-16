#ifndef QUEUE_H
#define QUEUE_H

#include "iter.h"
#include "node.h"
#include <stdbool.h>

struct queue
{
  struct node head;
  struct node tail;
};

#define QUEUE_INIT(name)                                                       \
  {                                                                            \
    {&name.head}, { &name.head }                                               \
  }

static inline struct iter queue_iter(struct queue const *x)
{
  return (struct iter){&x->head, x->tail.next, &x->head};
}

static inline void queue_put(struct queue *x, struct node *novel)
{
  struct node *a = x->tail.next;
  struct node *b = x->tail.next->next;
  struct node *c = novel;
  struct node *tmp = b->next;

  c->next = b;
  b->next = a;
  a->next = c;

  x->tail.next = a->next->next->next;

  b->next = tmp;
  a->next = c->next;

  struct node *next = x->head.next;
  next->next = novel;
  novel->next = &x->head;
  x->head.next = novel;
}

#endif
