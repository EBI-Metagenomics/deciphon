#ifndef ARGMAX_NEON_H
#define ARGMAX_NEON_H

#include "argmax_generic.h"
#include "compiler.h"
#include "find.h"
#include "vmax.h"

PURE int argmax2(float *dval, float const x[restrict])
{
  return argmax(dval, 2, x);
}

PURE int argmax3(float *dval, float const x[restrict])
{
  float val = vmax3(x);
  int i = find3(val, x);
  *dval = val;
  return i;
}

PURE int argmax5(float *dval, float const x[restrict])
{
  float val = vmax5(x);
  int i = find5(val, x);
  *dval = val;
  return i;
}

PURE int argmax10(float *dval, float const x[restrict])
{
  float val = vmax10(x);
  int i = find10(val, x);
  *dval = val;
  return i;
}

PURE int argmax20(float *dval, float const x[restrict])
{
  float val = vmax20(x);
  int i = find20(val, x);
  *dval = val;
  return i;
}

#endif
