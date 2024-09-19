#ifndef BIT_H
#define BIT_H

#include "compiler_attributes.h"
#include <limits.h>

ALWAYS_INLINE ATTRIBUTE_CONST
static unsigned bit_lowset(int n)
{
  return (n >= (int)sizeof(unsigned) * CHAR_BIT) ? (unsigned)-1 : (1U << n) - 1;
}

ALWAYS_INLINE ATTRIBUTE_CONST
static unsigned bit_extract(unsigned x, int low, int size)
{
  return (x >> low) & bit_lowset(size);
}

#endif
