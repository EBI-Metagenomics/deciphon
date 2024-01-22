#ifndef IPOW_H
#define IPOW_H

static inline int ipow(int const x, int const e)
{
  int r = 1;
  int xx = x;
  int ee = e;
  do
  {
    if (ee & 1) r *= xx;
    ee >>= 1;
    xx *= xx;
  } while (ee);
  return r;
}

#endif
