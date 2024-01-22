#ifndef IDOT_H
#define IDOT_H

static inline int idot(int size, int const x[], int const y[])
{
  int r = 0;
  for (int i = 0; i < size; ++i)
    r += x[i] * y[i];
  return r;
}

#endif
