#include "partition_size.h"
#include <assert.h>
#include <limits.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

static inline long ceildiv(long x, long y)
{
  assert(y > 0);
  assert(y - 1 <= UINT_MAX - x);
  return (x + y - 1) / y;
}

long partition_size(long nelems, long nparts, long idx)
{
  return ceildiv(MAX(0, nelems - idx), nparts);
}
