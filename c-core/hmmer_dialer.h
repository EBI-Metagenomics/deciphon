#ifndef DECIPHON_HMMER_DIALER_H
#define DECIPHON_HMMER_DIALER_H

struct dcp_hmmer;
struct h3client_dialer;

struct dcp_hmmer_dialer
{
  struct h3client_dialer *dialer;
};

int dcp_hmmer_dialer_init(struct dcp_hmmer_dialer *, int port);
void dcp_hmmer_dialer_cleanup(struct dcp_hmmer_dialer *);
int dcp_hmmer_dialer_dial(struct dcp_hmmer_dialer *, struct dcp_hmmer *);

#endif
