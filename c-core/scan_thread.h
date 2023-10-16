#ifndef SCAN_THREAD_H
#define SCAN_THREAD_H

#include "chararray.h"
#include "hmmer.h"
#include "protein.h"
#include "protein_iter.h"
#include "queue.h"
#include "viterbi.h"
#include <stdio.h>

struct chararray;
struct product_thread;

struct scan_thread
{
  struct protein protein;
  struct protein_iter iter;

  bool multi_hits;
  bool hmmer3_compat;
  bool disable_hmmer;

  struct viterbi_task task;
  struct product_thread *product;
  struct chararray amino;
  struct hmmer hmmer;
};

struct hmmer_dialer;
struct protein_reader;
struct seq;

struct scan_thread_params
{
  struct protein_reader *reader;
  int partition;
  struct product_thread *prod_thrd;
  struct hmmer_dialer *dialer;
  bool multi_hits;
  bool hmmer3_compat;
  bool disable_hmmer;
};

void scan_thread_init(struct scan_thread *);
int scan_thread_setup(struct scan_thread *, struct scan_thread_params);
void scan_thread_cleanup(struct scan_thread *);
int scan_thread_run(struct scan_thread *, struct queue const *seqs);

#endif
