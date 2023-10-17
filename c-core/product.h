#ifndef PRODUCT_H
#define PRODUCT_H

#include "product_thread.h"
#include "size.h"

struct product
{
  char dirname[DCP_PATH_MAX];
  int nthreads;
  struct product_thread threads[DCP_NTHREADS_MAX];
};

// clang-format off
void                   product_init(struct product *);
int                    product_open(struct product *, int nthreads,
                                    char const *dirname);
int                    product_close(struct product *);
struct product_thread *product_thread(struct product *, int idx);
// clang-format on

#endif
