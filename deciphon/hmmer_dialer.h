#ifndef HMMER_DIALER_H
#define HMMER_DIALER_H

struct hmmer;
struct h3client_dialer;

struct hmmer_dialer
{
  struct h3client_dialer *dialer;
};

int hmmer_dialer_init(struct hmmer_dialer *, int port);
void hmmer_dialer_cleanup(struct hmmer_dialer *);

int hmmer_dialer_dial(struct hmmer_dialer *, struct hmmer *);

#endif
