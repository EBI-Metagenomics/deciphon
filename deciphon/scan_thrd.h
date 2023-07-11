#ifndef DECIPHON_SCAN_THRD_H
#define DECIPHON_SCAN_THRD_H

#include "chararray.h"
#include "hmmer.h"
#include "protein_iter.h"
#include "scan_task.h"
#include <stdio.h>

struct chararray;
struct dcp_prod_writer_thrd;

struct scan_thrd
{
  struct dcp_protein protein;
  struct dcp_proteiniter iter;

  double lrt_threshold;
  bool multi_hits;
  bool hmmer3_compat;

  struct dcp_prod_writer_thrd *prod_thrd;
  struct chararray amino;
  struct dcp_hmmer hmmer;
};

struct hmmer_dialer;
struct prod_thrd;
struct dcp_protein_reader;
struct iseq;

int scan_thrd_init(struct scan_thrd *, struct dcp_protein_reader *,
                   int partition, struct dcp_prod_writer_thrd *,
                   struct hmmer_dialer *);
void scan_thrd_cleanup(struct scan_thrd *);

void scan_thrd_set_lrt_threshold(struct scan_thrd *, double lrt);
void scan_thrd_set_multi_hits(struct scan_thrd *, bool multihits);
void scan_thrd_set_hmmer3_compat(struct scan_thrd *, bool h3compat);
int scan_thrd_run(struct scan_thrd *, struct iseq const *);
int scan_thrd_run0(struct scan_thrd *, struct iseq const *);

#endif
