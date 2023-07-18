#include "partition_size.h"
#include "max.h"
#include <assert.h>
#include <limits.h>

static inline long ceildiv(long x, long y)
{
  assert(y > 0);
  assert(y - 1 <= UINT_MAX - x);
  return (x + y - 1) / y;
}

long dcp_partition_size(long nelems, long nparts, long idx)
{
  return ceildiv(dcp_max(0, nelems - idx), nparts);
}
