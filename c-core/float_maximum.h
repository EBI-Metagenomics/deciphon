#ifndef FLOAT_MAXIMUM_H
#define FLOAT_MAXIMUM_H

#include "compiler.h"
#include <math.h>

DCP_CONST float float_maximum(float a, float b)
{
#if __aarch64__
  return fmax(a, b);
#else
  return a > b ? a : b;
#endif
}

#endif
