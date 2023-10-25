#ifndef VITERBI_COREDP_H
#define VITERBI_COREDP_H

#include "compiler.h"
#include "xlimits.h"

#define DECLARE_COREDP(name) float *name

// clang-format off
void         coredp_init(float **x);
int          coredp_setup(float **, int core_size);
void         coredp_cleanup(float **);
CONST float *dp_core_next(float *x) { return x + 3 * DCP_PAST_SIZE; }
// clang-format on

#endif
