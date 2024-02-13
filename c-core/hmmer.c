#include "hmmer.h"
#include "error.h"
#include "h3client/deadline.h"
#include "h3client/errno.h"
#include "h3client/stream.h"
#include "hmmer_dialer.h"
#include "hmmer_result.h"

#define NUM_RETRIES 30
#define DEADLINE 30000

void hmmer_init(struct hmmer *x)
{
  x->cut_ga = true;
  x->stream = NULL;
  hmmer_result_init(&x->result);
}

int hmmer_setup(struct hmmer *x, bool cut_ga)
{
  x->cut_ga = cut_ga;
  return hmmer_result_setup(&x->result);
}

bool hmmer_online(struct hmmer const *x) { return x->stream; }

void hmmer_cleanup(struct hmmer *x)
{
  if (x)
  {
    if (x->stream) h3client_stream_del(x->stream);
    x->stream = NULL;
    hmmer_result_cleanup(&x->result);
  }
}

int hmmer_warmup(struct hmmer *x)
{
  char cmd[] = "--hmmdb 1 --hmmdb_range 0..0 --acc";
  long deadline = h3client_deadline(DEADLINE);

  if (h3client_stream_put(x->stream, cmd, "warmup", "", deadline))
    return error(DCP_EH3CWARMUP);

  h3client_stream_wait(x->stream);
  return h3client_stream_pop(x->stream, x->result.handle)
             ? error(DCP_EH3CWARMUP)
             : 0;
}

int hmmer_get(struct hmmer *x, int hmmidx, char const *name, char const *seq)
{
  char cmd[128] = {0};
  if (x->cut_ga)
    snprintf(cmd, sizeof(cmd), "--hmmdb 1 --hmmdb_range %d..%d --acc --cut_ga",
             hmmidx, hmmidx);
  else
    snprintf(
        cmd, sizeof(cmd),
        "--hmmdb 1 --hmmdb_range %d..%d --acc --incdomE e10-5 --incE e10-5",
        hmmidx, hmmidx);

  for (int i = 0; i < NUM_RETRIES; ++i)
  {
    long deadline = h3client_deadline(DEADLINE);

    int rc = h3client_stream_put(x->stream, cmd, name, seq, deadline);
    if (rc == H3CLIENT_ETIMEDOUT) continue;
    if (rc) return error(DCP_EH3CPUT);

    h3client_stream_wait(x->stream);
    rc = h3client_stream_pop(x->stream, x->result.handle);
    if (rc == H3CLIENT_ETIMEDOUT) continue;
    if (rc) return error(DCP_EH3CPOP);

    return 0;
  }

  return error(DCP_EH3CMAXRETRY);
}
