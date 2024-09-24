#ifndef WORK_H
#define WORK_H

#include "decoder.h"
#include "xtrans.h"
#include <stdbool.h>

struct protein;

struct work
{
  struct xtrans xtrans;
  bool multi_hits;
  bool hmmer3_compat;
  int core_size;
  char accession[32];
  struct decoder decoder;
  struct viterbi *viterbi;
};

void work_init(struct work *);
int  work_setup(struct work *, struct protein *);
void work_reset(struct work *, int seq_size);
void work_cleanup(struct work *);

#endif
