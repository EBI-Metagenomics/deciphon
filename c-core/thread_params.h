#ifndef THREAD_PARAMS_H
#define THREAD_PARAMS_H

#include <stdbool.h>

struct protein_reader;
struct product_thread;
struct hmmer_dialer;
struct scan_params;

struct thread_params
{
  struct protein_reader *reader;
  int partition;
  struct product_thread *product_thread;
  struct hmmer_dialer *dialer;
  bool multi_hits;
  bool hmmer3_compat;
};

void thread_params_init(struct thread_params *, struct scan_params *,
                        struct hmmer_dialer *, struct protein_reader *,
                        struct product_thread *, int partition);

#endif
