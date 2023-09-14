#ifndef DECIPHON_FIND_H
#define DECIPHON_FIND_H

#include "compiler.h"
#include <math.h>

DCP_CONST int find_fmax(int const size, float const x[restrict])
{
  int maxi = 0;
  float max = x[maxi];
  for (int i = 1; i < size; ++i)
  {
    if (max > x[i])
    {
      maxi = i;
      max = x[maxi];
    }
  }
  return maxi;
}

#endif
