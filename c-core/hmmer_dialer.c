#include "hmmer_dialer.h"
#include "h3client/h3client.h"
#include "hmmer.h"
#include "rc.h"
#include <stdlib.h>

#define DIAL_DEADLINE 15000

int dcp_hmmer_dialer_init(struct dcp_hmmer_dialer *x, int port)
{
  return (x->dialer = h3client_dialer_new("127.0.0.1", port)) ? 0 : DCP_ENOMEM;
}

void dcp_hmmer_dialer_cleanup(struct dcp_hmmer_dialer *x)
{
  if (x->dialer) h3client_dialer_del(x->dialer);
  x->dialer = NULL;
}

int dcp_hmmer_dialer_dial(struct dcp_hmmer_dialer *x, struct dcp_hmmer *y)
{
  if (h3client_dialer_dial(x->dialer, h3client_deadline(DIAL_DEADLINE)))
    return DCP_EH3CDIAL;
  y->stream = h3client_dialer_stream(x->dialer);
  return 0;
}
