#ifndef LRT_H
#define LRT_H

#include "compiler.h"

ATTRIBUTE_CONST float lrt(float null_loglik, float alt_loglik)
{
  return -2 * (null_loglik - alt_loglik);
}

#endif
