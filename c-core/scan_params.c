#include "scan_params.h"
#include "rc.h"
#include "size.h"

int scan_params_setup(struct scan_params *x, int num_threads, bool multi_hits,
                      bool hmmer3_compat, bool disable_hmmer)
{
  if (num_threads > DCP_NTHREADS_MAX) return DCP_EMANYTHREADS;
  x->num_threads = num_threads;
  x->multi_hits = multi_hits;
  x->hmmer3_compat = hmmer3_compat;
  x->disable_hmmer = disable_hmmer;
  return 0;
}
