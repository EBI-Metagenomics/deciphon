#ifndef DECIPHON_ITER_H
#define DECIPHON_ITER_H

#include "container.h"
#include "node.h"

struct iter
{
  struct node const *prev;
  struct node *curr;
  struct node const *end;
};

static inline struct node *iter_next(struct iter *x)
{
  if (x->curr == x->end) return NULL;
  struct node *node = x->curr;
  x->curr = node->next;
  return node;
}

#define iter_next_entry(iter, entry, member)                                   \
  container_of_safe(iter_next(iter), __typeof__(*entry), member)

#define iter_for_each_entry(entry, iter, member)                               \
  for (entry = iter_next_entry(iter, entry, member); entry;                    \
       entry = iter_next_entry(iter, entry, member))

#define iter_for_each_entry_safe(entry, tmp, iter, member)                     \
  for (entry = iter_next_entry(iter, entry, member),                           \
      tmp = iter_next_entry(iter, entry, member);                              \
       entry; entry = tmp, tmp = iter_next_entry(iter, entry, member))

#endif
