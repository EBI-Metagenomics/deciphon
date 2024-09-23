#include "scan.h"
#include "batch.h"
#include "database_reader.h"
#include "debug.h"
#include "defer_return.h"
#include "error.h"
#include "min.h"
#include "product.h"
#include "protein_reader.h"
#include "thread.h"
#include "thread_params.h"
#include "xsignal.h"
#include <stdlib.h>

#ifdef _OPENMP
#include <omp.h>
#else
static inline int omp_get_thread_num() { return 0; }
#endif

struct scan
{
  int port;
  int num_threads;
  bool multi_hits;
  bool hmmer3_compat;
  struct thread threads[THREAD_MAX];

  struct product product;

  struct
  {
    struct database_reader reader;
    struct protein_reader protein;
  } db;

  int total_proteins;
  int done_proteins;
  bool interrupted;
};

struct scan *scan_new(void)
{
  struct scan *x = malloc(sizeof(*x));
  if (!x) return NULL;

  x->port = -1;
  x->num_threads = 0;
  x->multi_hits = true;
  x->hmmer3_compat = false;

  product_init(&x->product);

  database_reader_init(&x->db.reader);
  protein_reader_init(&x->db.protein);

  x->total_proteins = 0;
  x->done_proteins = 0;
  x->interrupted = false;

  return x;
}

int scan_setup(struct scan *x, int port, int num_threads, bool multi_hits,
               bool hmmer3_compat)
{
  if (num_threads > THREAD_MAX) return error(DCP_EMANYTHREADS);

  x->port = port;
  x->num_threads = num_threads;
  x->multi_hits = multi_hits;
  x->hmmer3_compat = hmmer3_compat;

  for (int i = 0; i < x->num_threads; ++i)
    thread_init(x->threads + i);

  return 0;
}

void scan_del(struct scan const *scan)
{
  struct scan *x = (struct scan *)scan;
  if (x)
  {
    database_reader_close(&x->db.reader);
    product_close(&x->product);
    for (int i = 0; i < x->num_threads; ++i)
      thread_cleanup(x->threads + i);
    free(x);
  }
}

int scan_open(struct scan *x, char const *dbfile)
{
  int rc = 0;
  int n = x->num_threads;
  x->done_proteins = 0;

  if ((rc = database_reader_open(&x->db.reader, dbfile))) return rc;
  if ((rc = protein_reader_setup(&x->db.protein, &x->db.reader, n))) return rc;

  int abc = x->db.reader.code.super.abc->typeid;
  if (!(abc == IMM_DNA || abc == IMM_RNA)) return error(DCP_ENUCLTNOSUPPORT);

  x->total_proteins = protein_reader_size(&x->db.protein);

  for (int i = 0; i < n; ++i)
  {
    struct thread_params params = {&x->db.protein, i, x->port, x->multi_hits,
                                   x->hmmer3_compat};
    if ((rc = thread_setup(x->threads + i, params))) return rc;
  }

  return rc;
}

int scan_close(struct scan *x)
{
  return database_reader_close(&x->db.reader) ? error(DCP_EFCLOSE) : 0;
}

int scan_run(struct scan *x, struct batch *batch, char const *product_dir, bool (*interrupt)(void *),
             void *userdata)
{
  int rc = 0;
  x->done_proteins = 0;
  int n = x->num_threads;
  n = min(n, protein_reader_num_partitions(&x->db.protein));
  x->interrupted = false;
  struct xsignal *xsignal = NULL;
  if (!interrupt && !(xsignal = xsignal_new())) defer_return(error(DCP_ENOMEM));

  debug("%d thread(s)", n);
  if ((rc = batch_encode(batch, &x->db.reader.code.super))) defer_return(rc);

  if ((rc = product_open(&x->product, x->num_threads, product_dir))) defer_return(rc);

#pragma omp parallel for default(none)                                         \
    shared(x, n, rc, xsignal, interrupt, userdata, batch)
  for (int i = 0; i < n; ++i)
  {
    int *proteins = &x->done_proteins;
    int rank = omp_get_thread_num();
    struct xsignal *signal = rank == 0 ? xsignal : NULL;
    bool (*callb)(void *) = rank == 0 ? interrupt : NULL;
    void *args = rank == 0 ? userdata : NULL;
    int r = thread_run(x->threads + i, batch, proteins, signal, callb, args,
                       product_thread(&x->product, i));
    if (r || (rank == 0 && thread_interrupted(&x->threads[i])))
    {
      for (int i = 0; i < n; ++i)
        thread_interrupt(&x->threads[i]);
    }

#pragma omp critical
    if (r && !rc) rc = r;
  }

defer:
  xsignal_del(xsignal);

  for (int i = 0; i < x->num_threads; ++i)
    x->interrupted |= thread_interrupted(x->threads + i);

  int product_rc = product_close(&x->product);
  return rc ? rc : product_rc;
}

bool scan_interrupted(struct scan const *x) { return x->interrupted; }

int scan_progress(struct scan const *x)
{
  return (100 * x->done_proteins) / x->total_proteins;
}
