#ifndef MAXIMUM_H
#define MAXIMUM_H

#include "compiler.h"
#include <math.h>

ATTRIBUTE_CONST float maximum(float const a, float const b)
{
#if __aarch64__
  return fmax(a, b);
#else
  return a > b ? a : b;
#endif
}

#endif
