#ifndef NEAR_H
#define NEAR_H

#include <math.h>

static inline int nearf(float a, float b)
{
  static float const rel_tol = 1e-06f;
  if (a == b) return 1;
  if (isinf(a) || isinf(b)) return 0;
  float diff = fabsf(b - a);
  return (diff <= fabsf(rel_tol * b)) || (diff <= fabsf(rel_tol * a));
}

static inline int neard(double a, double b)
{
  static double const rel_tol = 1e-09;
  if (a == b) return 1;
  if (isinf(a) || isinf(b)) return 0;
  double diff = fabs(b - a);
  return (diff <= fabs(rel_tol * b)) || (diff <= fabs(rel_tol * a));
}

#define near(a, b) _Generic((a), float : nearf, double : neard)(a, b)

#endif
