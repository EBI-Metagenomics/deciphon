#ifndef BIT_H
#define BIT_H

#include "compiler.h"
#include <assert.h>
#include <limits.h>

// Set the first n bits to 1 from the lowest to the highest bits.
CONST unsigned bit_lowset(int n)
{
  assert(0 <= n && n <= (int)sizeof(unsigned) * CHAR_BIT);
  return (n >= (int)sizeof(unsigned) * CHAR_BIT) ? (unsigned)-1 : (1U << n) - 1;
}

// Set bits to 1 within range [low, low + size).
CONST unsigned bit_rangeset(int low, int size)
{
  assert(0 <= low && low <= (int)sizeof(unsigned) * CHAR_BIT);
  return bit_lowset(size) << low;
}

#define bit_extract(x, low, size)                                              \
  ({                                                                           \
    __typeof__(x) y = (x);                                                     \
    unsigned l = (low);                                                        \
    unsigned s = (size);                                                       \
    assert(l <= sizeof(y) * CHAR_BIT);                               \
    (y >> l) & bit_lowset(s);                                                  \
  })

#endif
