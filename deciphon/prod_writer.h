#ifndef DECIPHON_PROD_WRITER_H
#define DECIPHON_PROD_WRITER_H

#include "prod_writer_thrd.h"
#include "size.h"

struct dcp_prod_writer
{
  char dirname[DCP_SHORT_PATH_MAX];
  int nthreads;
  struct dcp_prod_writer_thrd threads[DCP_NTHREADS_MAX];
};

void dcp_prod_writer_init(struct dcp_prod_writer *);
int dcp_prod_writer_open(struct dcp_prod_writer *, int nthreads,
                         char const *dirname);
int dcp_prod_writer_close(struct dcp_prod_writer *);
struct dcp_prod_writer_thrd *dcp_prod_writer_thrd(struct dcp_prod_writer *,
                                                  int idx);

#endif
