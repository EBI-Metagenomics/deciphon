#include "scan.h"
#include "batch.h"
#include "database_reader.h"
#include "debug.h"
#include "defer_return.h"
#include "error.h"
#include "hmmer.h"
#include "imm_abc.h"
#include "min.h"
#include "product.h"
#include "product_thread.h"
#include "protein_iter.h"
#include "protein_reader.h"
#include "thread.h"
#include "workload.h"
#include "xsignal.h"
#include <stdlib.h>
#include <string.h>

#ifdef _OPENMP
#include <omp.h>
#else
static inline int omp_get_thread_num() { return 0; }
#endif

struct scan
{
  struct xsignal *xsignal;
  int num_threads;

  struct database_reader database_reader;
  struct protein_reader protein_reader;
  struct product product;

  struct protein proteins[THREAD_MAX];
  struct protein_iter protein_iters[THREAD_MAX];
  struct hmmer hmmers[THREAD_MAX];
  struct product_thread product_threads[THREAD_MAX];
  struct workload workloads[THREAD_MAX];
  struct thread threads[THREAD_MAX];

  int done_proteins;
  bool interrupted;
  void (*callback)(void *);
  void *userdata;
};

struct scan *scan_new(void)
{
  struct scan *x = malloc(sizeof(*x));
  if (!x) return NULL;

  if (!(x->xsignal = xsignal_new()))
  {
    free(x);
    return NULL;
  }
  x->num_threads = 0;

  database_reader_init(&x->database_reader);
  protein_reader_init(&x->protein_reader);
  product_init(&x->product);

  for (int i = 0; i < THREAD_MAX; ++i)
  {
    protein_init(x->proteins + i);
    protein_iter_init(x->protein_iters + i);
    hmmer_init(x->hmmers + i);
    product_thread_init(x->product_threads + i, i);
    workload_init(x->workloads + i);
    thread_init(x->threads + i);
  }

  x->done_proteins = 0;
  x->interrupted = false;
  x->callback = NULL;
  x->userdata = NULL;

  return x;
}

void scan_del(struct scan const *scan)
{
  struct scan *x = (struct scan *)scan;
  if (x)
  {
    xsignal_del(x->xsignal);
    database_reader_cleanup(&x->database_reader);
    protein_reader_cleanup(&x->protein_reader);
    for (int i = 0; i < x->num_threads; ++i)
    {
      struct protein        *protein  = x->proteins + i;
      struct hmmer          *hmmer    = x->hmmers + i;
      struct workload       *workload = x->workloads + i;
      struct thread         *thread   = x->threads + i;

      protein_cleanup(protein);
      hmmer_cleanup(hmmer);
      workload_cleanup(workload);
      thread_cleanup(thread);
    }
    free(x);
  }
}

int scan_setup(struct scan *x, char const *dbfile, int port, int num_threads,
               bool multi_hits, bool hmmer3_compat, bool cache,
               void (*callback)(void *), void *userdata)
{
  if (num_threads > THREAD_MAX) return error(DCP_EMANYTHREADS);

  int rc = 0;

  struct database_reader *db = &x->database_reader;

  if ((rc = database_reader_open(db, dbfile))) return rc;
  x->num_threads = min(num_threads, db->num_proteins);

  if ((rc = protein_reader_setup(&x->protein_reader, db, x->num_threads)))
    return rc;

  int abc = db->code.super.abc->typeid;
  if (!(abc == IMM_DNA || abc == IMM_RNA)) return error(DCP_ENUCLTNOSUPPORT);

  for (int i = 0; i < x->num_threads; ++i)
  {
    struct model_params    params   = database_reader_params(db, NULL);
    struct protein        *protein  = x->proteins + i;
    struct protein_iter   *it       = x->protein_iters + i;
    struct hmmer          *hmmer    = x->hmmers + i;
    struct workload       *workload = x->workloads + i;
    struct thread         *thread   = x->threads + i;

    protein_setup(protein, params, multi_hits, hmmer3_compat);
    if ((rc = protein_reader_iter(&x->protein_reader, i, it)))                 return rc;
    if ((rc = hmmer_setup(hmmer, db->has_ga, db->num_proteins, port)))         return rc;
    if ((rc = workload_setup(workload, cache, db->num_proteins, protein, it))) return rc;
    thread_setup(thread, hmmer, workload);
  }

  x->callback = callback;
  x->userdata = userdata;
  return database_reader_close(db);
}

int scan_run(struct scan *x, struct batch *batch, char const *product_dir)
{
  debug("%d thread(s)", x->num_threads);

  int rc = 0;

  x->done_proteins = 0;
  x->interrupted = false;

  struct imm_code *code = &x->database_reader.code.super;

  if ((rc = xsignal_set(x->xsignal)))                defer_return(rc);
  if ((rc = batch_encode(batch, code)))              defer_return(rc);
  if ((rc = product_open(&x->product, product_dir))) defer_return(rc);

  for (int i = 0; i < x->num_threads; ++i)
  {
    struct product_thread *product = x->product_threads + i;
    struct database_reader *db = &x->database_reader;
    char const *abc = imm_abc_typeid_name(db->code.super.abc->typeid);
    if ((rc = product_thread_setup(product, abc, product_dir))) defer_return(rc);
  }

#pragma omp parallel for
  for (int i = 0; i < x->num_threads; ++i)
  {
    struct thread         *thread   = x->threads + i;
    struct product_thread *product  = x->product_threads + i;
    int                   *proteins = &x->done_proteins;

    int rank                = omp_get_thread_num();
    struct xsignal *xsignal = rank == 0 ? x->xsignal   : NULL;
    void (*callb)(void *)   = rank == 0 ? x->callback  : NULL;
    void *args              = rank == 0 ? x->userdata  : NULL;

    int r = thread_run(thread, batch, proteins, xsignal, callb, args, product);
    if (r || (rank == 0 && thread_interrupted(&x->threads[i])))
    {
      for (int i = 0; i < x->num_threads; ++i)
        thread_interrupt(&x->threads[i]);
    }

#pragma omp critical
    if (r && !rc) rc = r;
  }

defer:
  xsignal_unset(x->xsignal);

  for (int i = 0; i < x->num_threads; ++i)
    x->interrupted |= thread_interrupted(x->threads + i);

  int product_rc = product_close(&x->product, x->num_threads);
  return rc ? rc : product_rc;
}

bool scan_interrupted(struct scan const *x) { return x->interrupted; }

int scan_progress(struct scan const *x)
{
  return (100 * x->done_proteins) / x->database_reader.num_proteins;
}
