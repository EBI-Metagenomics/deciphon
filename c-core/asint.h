#ifndef ASINT_H
#define ASINT_H

#include "compiler.h"

CONST int asint(float const x)
{
  return (union {
           float f;
           int i;
         }){.f = x}
      .i;
}

#endif
