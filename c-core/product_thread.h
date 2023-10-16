#ifndef PRODUCT_THREAD_H
#define PRODUCT_THREAD_H

#include "product_line.h"
#include <stdio.h>

struct match;
struct match_iter;
struct hmmer_result;

struct product_thread
{
  int idx;
  char const *dirname;
  char prodname[DCP_PATH_MAX];
  struct product_line line;
};

int product_thread_init(struct product_thread *, int idx, char const *dirname);
int product_thread_put(struct product_thread *, struct match *,
                       struct match_iter *);
int product_thread_put_hmmer(struct product_thread *,
                             struct hmmer_result const *);

#endif
