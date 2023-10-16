#ifndef SCAN_PARAMS_H
#define SCAN_PARAMS_H

#include <stdbool.h>

struct scan_params
{
  int num_threads;
  bool multi_hits;
  bool hmmer3_compat;
  bool disable_hmmer;
};

#endif
