#ifndef HMMER_H
#define HMMER_H

#include "hmmer_result.h"

struct hmmer
{
  struct h3client_stream *stream;
  struct hmmer_result result;
};

int hmmer_init(struct hmmer *);
void hmmer_cleanup(struct hmmer *);
int hmmer_warmup(struct hmmer *);
int hmmer_get(struct hmmer *, int hmmidx, char const *name, char const *seq);

#endif
