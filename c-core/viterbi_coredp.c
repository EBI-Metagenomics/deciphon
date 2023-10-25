#include "viterbi_coredp.h"
#include "rc.h"
#include "xrealloc.h"
#include <math.h>
#include <stddef.h>
#include <stdlib.h>

void coredp_init(float **x) { *x = NULL; }

int coredp_setup(float **x, int core_size)
{
  size_t size = 3 * DCP_PAST_SIZE * core_size;

  *x = xrealloc(*x, sizeof(float) * size);
  if (!*x && size > 0) return DCP_ENOMEM;

  for (size_t i = 0; i < size; ++i)
    (*x)[i] = -INFINITY;

  return 0;
}

void coredp_cleanup(float **x)
{
  free(*x);
  *x = NULL;
}
