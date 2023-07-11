#ifndef DECIPHON_PROD_WRITER_THRD_H
#define DECIPHON_PROD_WRITER_THRD_H

#include "prod_match.h"
#include <stdio.h>

struct dcp_match;
struct dcp_match_iter;
struct dcp_hmmer_result;

struct dcp_prod_writer_thrd
{
  int idx;
  char const *dirname;
  char prodname[DCP_SHORT_PATH_MAX];
  struct dcp_prod_match match;
};

int dcp_prod_writer_thrd_init(struct dcp_prod_writer_thrd *, int idx,
                              char const *dirname);
int dcp_prod_writer_thrd_put(struct dcp_prod_writer_thrd *, struct dcp_match *,
                             struct dcp_match_iter *);
int dcp_prod_writer_thrd_put_hmmer(struct dcp_prod_writer_thrd *,
                                   struct dcp_hmmer_result const *);

#endif
