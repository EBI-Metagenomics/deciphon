#include "hmmer_result.h"
#include "error.h"
#include "h3client/h3client.h"
#include "rc.h"
#include <math.h>
#include <stddef.h>

void hmmer_result_init(struct hmmer_result *x) { x->handle = NULL; }

int hmmer_result_setup(struct hmmer_result *x)
{
  if (x->handle) return 0;
  return (x->handle = h3client_result_new()) ? 0 : error(DCP_ENOMEM);
}

void hmmer_result_cleanup(struct hmmer_result *x)
{
  if (x)
  {
    if (x->handle) h3client_result_del(x->handle);
    x->handle = NULL;
  }
}

int hmmer_result_num_hits(struct hmmer_result const *x)
{
  return (int)h3client_result_nhits(x->handle);
}

float hmmer_result_evalue(struct hmmer_result const *x)
{
  if (hmmer_result_num_hits(x) == 0) return -INFINITY;
  return (float)exp(h3client_result_hit_evalue_ln(x->handle, 0));
}

int hmmer_result_pack(struct hmmer_result const *x, FILE *fp)
{
  return h3client_result_pack(x->handle, fp) ? error(DCP_EH3CPACK) : 0;
}
