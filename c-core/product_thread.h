#ifndef PRODUCT_THREAD_H
#define PRODUCT_THREAD_H

#include "fs.h"
#include "product_line.h"
#include "match.h"

struct h3r;

struct product_thread
{
  int id;
  char const *dirname;
  char filename[FS_PATH_MAX];
  struct product_line line;
};

void product_thread_init(struct product_thread *, int id);
int  product_thread_setup(struct product_thread *, char const *abc, char const *dirname);
int  product_thread_add_match(struct product_thread *, struct match begin, struct match end);
int  product_thread_add_hmmer(struct product_thread *, struct h3r const *);

#endif
