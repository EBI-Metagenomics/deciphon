#ifndef DECIPHON_SCAN_PARAMS_H
#define DECIPHON_SCAN_PARAMS_H

#include <stdbool.h>

struct dcp_scan_params
{
  int num_threads;
  double lrt_threshold;
  bool multi_hits;
  bool hmmer3_compat;
};

static inline struct dcp_scan_params dcp_scan_params(int num_threads,
                                                     double lrt_threshold,
                                                     bool multi_hits,
                                                     bool hmmer3_compat)
{
  return (struct dcp_scan_params){num_threads, lrt_threshold, multi_hits,
                                  hmmer3_compat};
}

#endif
