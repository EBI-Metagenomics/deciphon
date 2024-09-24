#include "hmmer.h"
#include "deciphon.h"
#include "defer_return.h"
#include "error.h"
#include "h3c_errnum.h"
#include "h3c_socket.h"
#include "h3r_result.h"

#define NUM_RETRIES 30
#define TIMEOUT 30000

void hmmer_init(struct hmmer *x)
{
  x->cut_ga       = true;
  x->num_proteins = -1;
  x->port         = -1;
  x->socket       = NULL;
  x->result       = NULL;
}

int hmmer_setup(struct hmmer *x, bool cut_ga, int num_proteins, int port)
{
  int rc = 0;
  x->cut_ga = cut_ga;
  x->num_proteins = num_proteins;
  x->port = port;
  if (!x->socket && !(x->socket = h3c_socket_new())) defer_return(DCP_ENOMEM);
  if (!x->result && !(x->result = h3r_new())) defer_return(DCP_ENOMEM);

  if ((rc = hmmer_dial(x))) return rc;
  if ((rc = hmmer_warmup(x))) return rc;

  return rc;

defer:
  if (x->socket)
  {
    h3c_socket_del(x->socket);
    x->socket = NULL;
  }
  if (x->result)
  {
    h3r_del(x->result);
    x->result = NULL;
  }
  return rc;
}

int hmmer_dial(struct hmmer *x)
{
  if (h3c_socket_dial(x->socket, "127.0.0.1", x->port, TIMEOUT))
    return error(DCP_EH3CDIAL);
  return 0;
}

void hmmer_cleanup(struct hmmer *x)
{
  if (x)
  {
    if (x->socket)
    {
      h3c_socket_hangup(x->socket);
      h3c_socket_del(x->socket);
      x->socket = NULL;
    }
    if (x->result)
    {
      h3r_del(x->result);
      x->result = NULL;
    }
  }
}

int hmmer_warmup(struct hmmer *x)
{
  char cmd[] = "--hmmdb 1 --hmmdb_range 0..0 --acc";

  if (h3c_socket_send(x->socket, cmd, "")) return error(DCP_EH3CWARMUP);
  if (h3c_socket_recv(x->socket, x->result)) return error(DCP_EH3CWARMUP);
  return 0;
}

int hmmer_get(struct hmmer *x, int hmmidx, char const *seq)
{
  char cmd[128] = {0};
  if (x->cut_ga)
    snprintf(cmd, sizeof(cmd), "--hmmdb 1 --hmmdb_range %d..%d --acc --cut_ga",
             hmmidx, hmmidx);
  else
    snprintf(cmd, sizeof(cmd),
             "--hmmdb 1 --hmmdb_range %d..%d --acc -Z %d -E 1e-10", hmmidx,
             hmmidx, x->num_proteins);

  for (int i = 0; i < NUM_RETRIES; ++i)
  {
    int rc = h3c_socket_send(x->socket, cmd, seq);
    if (rc == H3C_ETIMEDOUT) continue;
    if (rc) return error(DCP_EH3CPUT);

    rc = h3c_socket_recv(x->socket, x->result);
    if (rc == H3C_ETIMEDOUT) continue;
    if (rc) return error(DCP_EH3CPOP);

    return 0;
  }

  return error(DCP_EH3CMAXRETRY);
}
