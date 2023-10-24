#ifndef PRODUCT_H
#define PRODUCT_H

#include "product_thread.h"
#include "xlimits.h"
#include <stdbool.h>

struct product
{
  char dirname[DCP_PATH_MAX];
  int num_threads;
  struct product_thread threads[DCP_NTHREADS_MAX];
  bool closed;
};

// clang-format off
void                   product_init(struct product *);
int                    product_open(struct product *, int num_threads, char const *dirname);
int                    product_close(struct product *);
struct product_thread *product_thread(struct product *, int thread_id);
// clang-format on

#endif
