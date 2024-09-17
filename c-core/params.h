#ifndef PARAMS_H
#define PARAMS_H

#include "api.h"
#include <stdbool.h>

struct params
{
  int port;
  int num_threads;
  bool multi_hits;
  bool hmmer3_compat;
};

API int params_setup(struct params *, int num_threads, bool multi_hits,
                     bool hmmer3_compat);

#endif
