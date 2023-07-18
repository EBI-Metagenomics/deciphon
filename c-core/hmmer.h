#ifndef DECIPHON_HMMER_H
#define DECIPHON_HMMER_H

#include "hmmer_result.h"

struct dcp_hmmer
{
  struct h3client_stream *stream;
  struct dcp_hmmer_result result;
};

int dcp_hmmer_init(struct dcp_hmmer *);
void dcp_hmmer_cleanup(struct dcp_hmmer *);
int dcp_hmmer_warmup(struct dcp_hmmer *);
int dcp_hmmer_get(struct dcp_hmmer *, int hmmidx, char const *name,
                  char const *seq);

#endif
