#ifndef DECIPHON_PROD_WRITER_H
#define DECIPHON_PROD_WRITER_H

#include "prod_writer_thrd.h"
#include "size.h"

struct prod_writer
{
  char dirname[DCP_SHORT_PATH_MAX];
  int nthreads;
  struct prod_writer_thrd threads[DCP_NTHREADS_MAX];
};

void prod_writer_init(struct prod_writer *);
int prod_writer_open(struct prod_writer *, int nthreads, char const *dirname);
int prod_writer_close(struct prod_writer *);
struct prod_writer_thrd *prod_writer_thrd(struct prod_writer *, int idx);

#endif
