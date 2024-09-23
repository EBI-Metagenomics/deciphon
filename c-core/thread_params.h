#ifndef THREAD_PARAMS_H
#define THREAD_PARAMS_H

#include <stdbool.h>

struct protein_reader;
struct params;

struct thread_params
{
  struct protein_reader *reader;
  int partition;
  int port;
  bool multi_hits;
  bool hmmer3_compat;
};

#endif
