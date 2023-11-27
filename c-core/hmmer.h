#ifndef HMMER_H
#define HMMER_H

#include "hmmer_result.h"
#include <stdbool.h>

struct hmmer
{
  bool cut_ga;
  struct h3client_stream *stream;
  struct hmmer_result result;
};

// clang-format off
void hmmer_init(struct hmmer *);
int  hmmer_setup(struct hmmer *, bool cut_ga);
bool hmmer_online(struct hmmer const *);
void hmmer_cleanup(struct hmmer *);
int  hmmer_warmup(struct hmmer *);
int  hmmer_get(struct hmmer *, int hmmidx, char const *name, char const *seq);
// clang-format on

#endif
