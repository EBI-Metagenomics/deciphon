#ifndef PRODUCT_H
#define PRODUCT_H

#include "fs.h"
#include <stdbool.h>

struct product
{
  char dirname[FS_PATH_MAX];
  bool closed;
};

void product_init(struct product *);
int  product_open(struct product *, char const *dirname);
int  product_close(struct product *, int num_threads);

#endif
