#include "params.h"
#include "error.h"
#include "rc.h"
#include "thread.h"

int params_setup(struct params *x, int num_threads, bool multi_hits,
                 bool hmmer3_compat)
{
  if (num_threads > THREAD_MAX) return error(DCP_EMANYTHREADS);
  x->num_threads = num_threads;
  x->multi_hits = multi_hits;
  x->hmmer3_compat = hmmer3_compat;
  return 0;
}
