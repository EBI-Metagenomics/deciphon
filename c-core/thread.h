#ifndef THREAD_H
#define THREAD_H

#include "chararray.h"
#include "hmmer.h"
#include "imm_path.h"
#include "protein.h"
#include "protein_iter.h"
#include "thread_params.h"

#define THREAD_MAX 128

struct product_thread;
struct batch;
struct viterbi;
struct xsignal;

struct thread
{
  struct protein protein;
  struct protein_iter iter;

  bool multi_hits;
  bool hmmer3_compat;

  char const *abc;
  struct viterbi *viterbi;
  int partition;
  struct chararray amino;
  struct hmmer hmmer;
  struct imm_path path;
  bool interrupted;
};

void thread_init(struct thread *);
int  thread_setup(struct thread *, struct thread_params);
void thread_cleanup(struct thread *);
int  thread_run(struct thread *, struct batch const *,
                int *done_proteins, struct xsignal *, bool (*interrupt)(void *),
                void (*userdata)(void *),
                struct product_thread *);
void thread_interrupt(struct thread *);
bool thread_interrupted(struct thread const *);

#endif
