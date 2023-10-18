#ifndef HMMER_DIALER_H
#define HMMER_DIALER_H

#include <stdbool.h>

struct hmmer;
struct h3client_dialer;

struct hmmer_dialer
{
  struct h3client_dialer *dialer;
};

void hmmer_dialer_init(struct hmmer_dialer *);
int hmmer_dialer_setup(struct hmmer_dialer *, int port);
bool hmmer_dialer_isset(struct hmmer_dialer const *);
void hmmer_dialer_cleanup(struct hmmer_dialer *);
int hmmer_dialer_dial(struct hmmer_dialer *, struct hmmer *);

#endif
