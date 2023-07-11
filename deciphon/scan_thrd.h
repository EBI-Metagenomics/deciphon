#ifndef DECIPHON_SCAN_THRD_H
#define DECIPHON_SCAN_THRD_H

#include "chararray.h"
#include "hmmer.h"
#include "protein_iter.h"
#include "scan_task.h"
#include <stdio.h>

struct dcp_chararray;
struct dcp_prod_writer_thrd;

struct dcp_scan_thrd
{
  struct dcp_protein protein;
  struct dcp_protein_iter iter;

  double lrt_threshold;
  bool multi_hits;
  bool hmmer3_compat;

  struct dcp_prod_writer_thrd *prod_thrd;
  struct dcp_chararray amino;
  struct dcp_hmmer hmmer;
};

struct hmmer_dialer;
struct prod_thrd;
struct dcp_protein_reader;
struct iseq;

int dcp_scan_thrd_init(struct dcp_scan_thrd *, struct dcp_protein_reader *,
                       int partition, struct dcp_prod_writer_thrd *,
                       struct hmmer_dialer *);
void dcp_scan_thrd_cleanup(struct dcp_scan_thrd *);

void dcp_scan_thrd_set_lrt_threshold(struct dcp_scan_thrd *, double lrt);
void dcp_scan_thrd_set_multi_hits(struct dcp_scan_thrd *, bool multihits);
void dcp_scan_thrd_set_hmmer3_compat(struct dcp_scan_thrd *, bool h3compat);
int dcp_scan_thrd_run(struct dcp_scan_thrd *, struct iseq const *);
int dcp_scan_thrd_run0(struct dcp_scan_thrd *, struct iseq const *);

#endif
