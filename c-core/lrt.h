#ifndef DECIPHON_LRT_H
#define DECIPHON_LRT_H

#include "compiler.h"

DCP_CONST float lrt(float null_loglik, float alt_loglik)
{
  return -2 * (null_loglik - alt_loglik);
}

#endif
