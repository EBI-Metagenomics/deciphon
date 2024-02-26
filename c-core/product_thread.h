#ifndef PRODUCT_THREAD_H
#define PRODUCT_THREAD_H

#include "fs.h"
#include "product_line.h"
#include "match.h"

struct hmmer_result;

struct product_thread
{
  int id;
  char const *dirname;
  char filename[FS_PATH_MAX];
  struct product_line line;
};

int product_thread_init(struct product_thread *, int thread_id, char const *dirname);
int product_thread_put_match(struct product_thread *, struct match begin, struct match end);
int product_thread_put_hmmer(struct product_thread *, struct hmmer_result const *);

#endif
