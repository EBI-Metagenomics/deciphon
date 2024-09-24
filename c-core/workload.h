#ifndef WORKLOAD_H
#define WORKLOAD_H

#include "work.h"
#include <stdbool.h>

struct protein;
struct protein_iter;

struct workload
{
  bool cache;
  int num_proteins;
  struct protein *protein;
  struct protein_iter *protein_iter;
  int index;
  struct work *works;
};

void workload_init(struct workload *);
int  workload_setup(struct workload *, bool cache, int num_proteins,
                    struct protein *, struct protein_iter *);
int  workload_rewind(struct workload *);
int  workload_next(struct workload *, struct work **);
bool workload_end(struct workload *);
int  workload_index(struct workload const *);
void workload_cleanup(struct workload *);

#endif
