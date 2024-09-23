#ifndef HMMER_H
#define HMMER_H

#include <stdbool.h>

struct h3c_socket;
struct h3r;

struct hmmer
{
  bool cut_ga;
  int num_proteins;
  int port;
  struct h3c_socket *socket;
  struct h3r        *result;
};

void hmmer_init(struct hmmer *);
int  hmmer_setup(struct hmmer *, bool cut_ga, int num_proteins, int port);
int  hmmer_dial(struct hmmer *);
void hmmer_cleanup(struct hmmer *);
int  hmmer_warmup(struct hmmer *);
int  hmmer_get(struct hmmer *, int hmmidx, char const *seq);

#endif
