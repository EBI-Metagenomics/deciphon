#ifndef DECIPHON_SCAN_THRD_H
#define DECIPHON_SCAN_THRD_H

#include "chararray.h"
#include "hmmer.h"
#include "protein.h"
#include "protein_iter.h"
#include "queue.h"
#include "viterbi.h"
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
  bool disable_hmmer;

  struct dcp_viterbi_task task;
  struct dcp_prod_writer_thrd *prod_thrd;
  struct dcp_chararray amino;
  struct dcp_hmmer hmmer;
};

struct dcp_hmmer_dialer;
struct dcp_protein_reader;
struct dcp_seq;

struct dcp_scan_thrd_params
{
  struct dcp_protein_reader *reader;
  int partition;
  struct dcp_prod_writer_thrd *prod_thrd;
  struct dcp_hmmer_dialer *dialer;
  double lrt_threshold;
  bool multi_hits;
  bool hmmer3_compat;
  bool disable_hmmer;
};

void dcp_scan_thrd_init(struct dcp_scan_thrd *);
int dcp_scan_thrd_setup(struct dcp_scan_thrd *, struct dcp_scan_thrd_params);
void dcp_scan_thrd_cleanup(struct dcp_scan_thrd *);
int dcp_scan_thrd_run(struct dcp_scan_thrd *, struct queue const *seqs);

#endif
