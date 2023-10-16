#ifndef NODE_H
#define NODE_H

#include <stddef.h>

struct node
{
  struct node *next;
};

static inline void node_init(struct node *node) { node->next = NULL; }

#endif
