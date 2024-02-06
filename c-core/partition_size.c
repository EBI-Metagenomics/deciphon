#include "partition_size.h"
#include "bug.h"
#include "max.h"
#include <limits.h>

static inline long ceildiv(long x, long y)
{
  BUG_ON(y <= 0);
  BUG_ON(y - 1 > UINT_MAX - x);
  return (x + y - 1) / y;
}

long partition_size(long nelems, long nparts, long idx)
{
  return ceildiv(max(0, nelems - idx), nparts);
}
