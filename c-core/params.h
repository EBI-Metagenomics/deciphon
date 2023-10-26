#ifndef PARAMS_H
#define PARAMS_H

#include <stdbool.h>

struct params
{
  int num_threads;
  bool multi_hits;
  bool hmmer3_compat;
};

int params_setup(struct params *, int num_threads, bool multi_hits,
                 bool hmmer3_compat);

#endif
