#include "hmmer.h"
#include "h3client/h3client.h"
#include "hmmer_dialer.h"
#include "hmmer_result.h"
#include "rc.h"

#define NUM_RETRIES 15
#define REQUEST_DEADLINE 30000
#define WARMUP_DEADLINE 10000

int dcp_hmmer_init(struct dcp_hmmer *x)
{
  x->stream = NULL;
  return dcp_hmmer_result_init(&x->result);
}

void dcp_hmmer_cleanup(struct dcp_hmmer *x)
{
  if (x)
  {
    if (x->stream) h3client_stream_del(x->stream);
    x->stream = NULL;
    dcp_hmmer_result_cleanup(&x->result);
  }
}

int dcp_hmmer_warmup(struct dcp_hmmer *x)
{
  char cmd[] = "--hmmdb 1 --hmmdb_range 0..0 --acc --cut_ga";
  long deadline = h3client_deadline(WARMUP_DEADLINE);

  if (h3client_stream_put(x->stream, cmd, "warmup", "", deadline))
    return DCP_EH3CWARMUP;

  h3client_stream_wait(x->stream);
  return h3client_stream_pop(x->stream, x->result.handle) ? DCP_EH3CWARMUP : 0;
}

int dcp_hmmer_get(struct dcp_hmmer *x, int hmmidx, char const *name,
                  char const *seq)
{
  char cmd[128] = {0};
  snprintf(cmd, sizeof(cmd), "--hmmdb 1 --hmmdb_range %d..%d --acc --cut_ga",
           hmmidx, hmmidx);

  for (int i = 0; i < NUM_RETRIES; ++i)
  {
    long deadline = h3client_deadline(REQUEST_DEADLINE);

    int rc = h3client_stream_put(x->stream, cmd, name, seq, deadline);
    if (rc == H3CLIENT_ETIMEDOUT) continue;
    if (rc) return DCP_EH3CPUT;

    h3client_stream_wait(x->stream);
    rc = h3client_stream_pop(x->stream, x->result.handle);
    if (rc == H3CLIENT_ETIMEDOUT) continue;
    if (rc) return DCP_EH3CPOP;

    return 0;
  }

  return DCP_EH3CMAXRETRY;
}
