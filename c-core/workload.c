#include "workload.h"
#include "bug.h"
#include "deciphon.h"
#include "defer_return.h"
#include "error.h"
#include "protein_iter.h"
#include <stddef.h>
#include <stdlib.h>

void workload_init(struct workload *x)
{
  x->cache        = false;
  x->num_proteins = -1;
  x->protein      = NULL;
  x->protein_iter = NULL;
  x->index        = -1;
  x->works        = NULL;
}

int workload_setup(struct workload *x, bool cache, int num_proteins,
                   struct protein *protein, struct protein_iter *iter)
{
  int rc          = 0;
  x->cache        = cache;
  x->num_proteins = num_proteins;
  x->protein      = protein;
  x->protein_iter = iter;
  x->index = -1;

  x->works = malloc(sizeof(*x->works) * (cache ? num_proteins : 1));
  if (!x->works) return error(DCP_ENOMEM);

  for (int i = 0; i < (cache ? num_proteins : 1); ++i)
    work_init(x->works + i);

  if ((rc = protein_iter_rewind(x->protein_iter))) defer_return(rc);
  if (cache)
  {
    while (!(rc = protein_iter_next(x->protein_iter, x->protein)))
    {
      if (protein_iter_end(x->protein_iter)) break;

      x->index += 1;
      if ((rc = work_setup(x->works + x->index, x->protein))) defer_return(rc);
    }
    if (rc) defer_return(rc);
    BUG_ON(x->index + 1 != num_proteins);
  }
  x->index = -1;

  return 0;

defer:
  if (x->cache)
  {
    for (int i = 0; i < x->index; ++i)
      work_cleanup(x->works + i);
  }
  free(x->works);
  return rc;
}

int workload_rewind(struct workload *x)
{
  x->index = -1;
  return x->cache ? 0 : protein_iter_rewind(x->protein_iter);
}

int workload_next(struct workload *x, struct work **work)
{
  int rc = 0;

  x->index += 1;
  if (workload_end(x)) return 0;

  *work = x->works + (x->cache ? x->index : 0);
  if (!x->cache)
  {
    if ((rc = protein_iter_next(x->protein_iter, x->protein))) return rc;
    if ((rc = work_setup(*work, x->protein)))                  return rc;
  }

  return 0;
}

bool workload_end(struct workload *x)
{
  return x->cache ? x->index == x->num_proteins
                  : protein_iter_end(x->protein_iter);
}

int workload_index(struct workload const *x) { return x->index; }

void workload_cleanup(struct workload *x)
{
  for (int i = 0; i < (x->cache ? x->num_proteins : 1); ++i)
    work_cleanup(x->works + i);
  free(x->works);
}
