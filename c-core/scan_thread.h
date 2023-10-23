#ifndef SCAN_THREAD_H
#define SCAN_THREAD_H

#include "chararray.h"
#include "hmmer.h"
#include "protein.h"
#include "protein_iter.h"
#include "queue.h"
#include "scan_thread_params.h"
#include "viterbi.h"

struct chararray;
struct product_thread;
struct sequence_queue;

struct scan_thread
{
  struct protein protein;
  struct protein_iter iter;

  bool multi_hits;
  bool hmmer3_compat;

  struct viterbi_task task;
  struct product_thread *product;
  struct chararray amino;
  struct hmmer hmmer;
};

struct hmmer_dialer;
struct sequence;

void scan_thread_init(struct scan_thread *);
int scan_thread_setup(struct scan_thread *, struct scan_thread_params);
void scan_thread_cleanup(struct scan_thread *);
int scan_thread_run(struct scan_thread *, struct sequence_queue const *);

#endif