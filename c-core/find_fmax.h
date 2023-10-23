#ifndef FIND_FMAX_H
#define FIND_FMAX_H

#include "compiler.h"

CONST int find_fmax(int const size, float const x[restrict])
{
  int maxi = 0;
  float max = x[maxi];
  for (int i = 1; i < size; ++i)
  {
    if (x[i] > max)
    {
      maxi = i;
      max = x[maxi];
    }
  }
  return maxi;
}

#endif
