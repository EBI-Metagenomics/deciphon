#ifndef FIND_GENERIC_H
#define FIND_GENERIC_H

#include "compiler.h"

PURE int find(float needle, int const size, float const stack[restrict])
{
  for (int i = 0; i < size; ++i)
  {
    if (stack[i] == needle) return i;
  }
  UNREACHABLE();
  return -1;
}

#endif
