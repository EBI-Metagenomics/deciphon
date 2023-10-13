#ifndef DECIPHON_TEST_SEQIT_H
#define DECIPHON_TEST_SEQIT_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct seq
{
  int id;
  char name[256];
  char const *data;
};

struct seqit
{
  struct seq seq;
  int size;
  char const *(*callb)(void);
};

static struct seqit seqit_init(int size, char const *(*callb)(void))
{
  return (struct seqit){{-1, {0}, NULL}, size, callb};
}

static struct seq const *seqit_next(struct seqit *x)
{
  if (x->size > 0)
  {
    x->size -= 1;
    x->seq.id += 1;
    snprintf(x->seq.name, 256, "name%d", x->seq.id);
    x->seq.data = (*x->callb)();
    return &x->seq;
  }
  return NULL;
}

#endif
