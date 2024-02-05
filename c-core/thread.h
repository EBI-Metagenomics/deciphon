#ifndef THREAD_H
#define THREAD_H

#include "chararray.h"
#include "hmmer.h"
#include "imm/path.h"
#include "protein.h"
#include "protein_iter.h"
#include "thread_params.h"

struct chararray;
struct product_thread;
struct sequence_queue;

struct thread
{
  struct protein protein;
  struct protein_iter iter;

  bool multi_hits;
  bool hmmer3_compat;

  struct vit *viterbi;
  struct product_thread *product;
  int partition;
  struct chararray amino;
  struct hmmer hmmer;
  struct imm_path path;
};

struct hmmer_dialer;
struct sequence;

// clang-format off
void thread_init(struct thread *);
int  thread_setup(struct thread *, struct thread_params);
void thread_cleanup(struct thread *);
int  thread_run(struct thread *, struct sequence_queue const *, int *done_proteins);
// clang-format on

#endif
