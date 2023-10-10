#ifndef DECIPHON_BIT_H
#define DECIPHON_BIT_H

#include "compiler.h"
#include <assert.h>
#include <limits.h>

// Set the first n bits to 1 from the lowest to the highest bits.
DCP_CONST unsigned bit_lowset(int n)
{
  assert(0 <= n && n <= sizeof(unsigned) * CHAR_BIT);
  return ((unsigned)n >= sizeof(unsigned) * CHAR_BIT) ? (unsigned)-1
                                                      : (1U << n) - 1;
}

// Set bits to 1 within range [low, low + size).
DCP_CONST unsigned bit_rangeset(int low, int size)
{
  assert(0 <= low && low <= sizeof(unsigned) * CHAR_BIT);
  return bit_lowset(size) << low;
}

#define bit_extract(x, low, size)                                              \
  ({                                                                           \
    __typeof__(x) y = (x);                                                     \
    int l = (low);                                                             \
    int s = (size);                                                            \
    assert(0 <= l && l <= sizeof(y) * CHAR_BIT);                               \
    (y >> l) & bit_lowset(s);                                                  \
  })

#endif
