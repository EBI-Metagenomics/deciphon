#ifndef ISUM_H
#define ISUM_H

static inline int isum(int size, int const x[])
{
  int r = 0;
  for (int i = 0; i < size; ++i)
    r += x[i];
  return r;
}

#endif
