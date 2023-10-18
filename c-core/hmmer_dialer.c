#include "hmmer_dialer.h"
#include "h3client/h3client.h"
#include "hmmer.h"
#include "rc.h"
#include <stdlib.h>

#define DIAL_DEADLINE 15000

void hmmer_dialer_init(struct hmmer_dialer *x) { x->dialer = NULL; }

int hmmer_dialer_setup(struct hmmer_dialer *x, int port)
{
  return (x->dialer = h3client_dialer_new("127.0.0.1", port)) ? 0 : DCP_ENOMEM;
}

bool hmmer_dialer_isset(struct hmmer_dialer const *x) { return x->dialer; }

void hmmer_dialer_cleanup(struct hmmer_dialer *x)
{
  if (x->dialer) h3client_dialer_del(x->dialer);
  x->dialer = NULL;
}

int hmmer_dialer_dial(struct hmmer_dialer *x, struct hmmer *y)
{
  if (h3client_dialer_dial(x->dialer, h3client_deadline(DIAL_DEADLINE)))
    return DCP_EH3CDIAL;
  y->stream = h3client_dialer_stream(x->dialer);
  return 0;
}
