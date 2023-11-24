#ifndef ARGMAX_GENERIC_H
#define ARGMAX_GENERIC_H

#include "compiler.h"

PURE int argmax(float *dval, int const size, float const x[restrict])
{
  int arg = 0;
  float val = x[arg];
  for (int i = 1; i < size; ++i)
  {
    if (x[i] > val)
    {
      arg = i;
      val = x[arg];
    }
  }
  *dval = val;
  return arg;
}

#endif
