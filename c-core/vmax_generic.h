#ifndef VMAX_GENERIC_H
#define VMAX_GENERIC_H

#include "maximum.h"

PURE float vmax(int const size, float const x[restrict])
{
  float max = x[0];
  for (int i = 1; i < size; ++i)
    max = maximum(max, x[i]);
  return max;
}

#endif
