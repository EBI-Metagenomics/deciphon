#ifndef DECIPHON_FMAX_H
#define DECIPHON_FMAX_H

#include "compiler.h"

DCP_CONST float dcp_fmax(float a, float b)
{
#if __aarch64__
  __asm__("fmaxnm %s0, %s1, %s2" : "=w"(a) : "w"(a), "w"(b));
#else
  a = a > b ? a : b;
#endif
  return a;
}

#endif
