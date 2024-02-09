#ifndef THREAD_PARAMS_H
#define THREAD_PARAMS_H

#if !defined(_POSIX_C_SOURCE) || _POSIX_C_SOURCE < 200809L
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <signal.h>
#include <stdbool.h>

struct protein_reader;
struct product_thread;
struct hmmer_dialer;
struct params;

struct thread_params
{
  struct protein_reader *reader;
  int partition;
  struct product_thread *product_thread;
  struct hmmer_dialer *dialer;
  bool multi_hits;
  bool hmmer3_compat;
  sigset_t signal_mask;
};

#endif
