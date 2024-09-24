#ifndef THREAD_H
#define THREAD_H

#include "chararray.h"
#include "imm_path.h"

#define THREAD_MAX 128

struct product_thread;
struct dcp_batch;
struct viterbi;
struct hmmer;
struct workload;

struct thread
{
  struct workload *workload;
  struct chararray amino;
  struct hmmer *hmmer;
  struct imm_path path;
  bool interrupted;
};

void thread_init(struct thread *);
void thread_setup(struct thread *, struct hmmer *, struct workload *);
void thread_cleanup(struct thread *);
int  thread_run(struct thread *, struct dcp_batch const *,
                int *done_proteins, void (callb)(void *),
                void (*userdata)(void *),
                struct product_thread *);
void thread_interrupt(struct thread *);
bool thread_interrupted(struct thread const *);

#endif
