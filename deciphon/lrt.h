#ifndef DECIPHON_LRT_H
#define DECIPHON_LRT_H

#include "compiler.h"

dcp_const_template double lrt(double null_loglik, double alt_loglik)
{
  return -2 * (null_loglik - alt_loglik);
}

#endif
