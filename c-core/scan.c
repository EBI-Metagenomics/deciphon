#include "batch.h"
#include "database_reader.h"
#include "debug.h"
#include "deciphon.h"
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
#include <omp.h>
#include <stdlib.h>
#include <string.h>

struct dcp_scan
{
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
  void (*callback)(void *);
  void *userdata;
};

struct dcp_scan *dcp_scan_new(void)
{
  struct dcp_scan *x = malloc(sizeof(*x));
  if (!x) return NULL;

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
  x->callback = NULL;
  x->userdata = NULL;

  return x;
}

void dcp_scan_del(struct dcp_scan const *scan)
{
  struct dcp_scan *x = (struct dcp_scan *)scan;
  if (x)
  {
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

int dcp_scan_setup(struct dcp_scan *x, char const *dbfile, int port, int num_threads,
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

#pragma omp parallel for
  for (int i = 0; i < x->num_threads; ++i)
  {
    struct model_params    params   = database_reader_params(db, NULL);
    struct protein        *protein  = x->proteins + i;
    struct protein_iter   *it       = x->protein_iters + i;
    struct hmmer          *hmmer    = x->hmmers + i;

    int r = 0;
    protein_setup(protein, params, multi_hits, hmmer3_compat);
    if ((r = protein_reader_iter(&x->protein_reader, i, it)))             goto loop_exit0;
    if ((r = hmmer_setup(hmmer, db->has_ga, db->num_proteins, port)))     goto loop_exit0;

loop_exit0:
#pragma omp critical
    if (r && !rc) rc = r;
  }

  if (rc)
  {
    database_reader_close(db);
    return rc;
  }

#pragma omp parallel for
  for (int i = 0; i < x->num_threads; ++i)
  {
    struct protein        *protein  = x->proteins + i;
    struct protein_iter   *it       = x->protein_iters + i;
    struct hmmer          *hmmer    = x->hmmers + i;
    struct workload       *workload = x->workloads + i;
    struct thread         *thread   = x->threads + i;
    int index_offset                = protein_reader_partition_cumsize(&x->protein_reader, i);
    int num_proteins                = protein_reader_partition_size(&x->protein_reader, i);

    int r = 0;
    if ((r = workload_setup(workload, cache, index_offset, num_proteins, protein, it))) goto loop_exit1;
    thread_setup(thread, hmmer, workload);

loop_exit1:
#pragma omp critical
    if (r && !rc) rc = r;
  }

  x->callback = callback;
  x->userdata = userdata;
  return database_reader_close(db);
}

int dcp_scan_run(struct dcp_scan *x, struct dcp_batch *batch, char const *product_dir)
{
  debug("%d thread(s)", x->num_threads);

  int rc = 0;

  x->done_proteins = 0;

  struct imm_code *code = &x->database_reader.code.super;

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
    void (*callb)(void *)   = rank == 0 ? x->callback  : NULL;
    void *args              = rank == 0 ? x->userdata  : NULL;

    int r = thread_run(thread, batch, proteins, callb, args, product);
    if (r || (rank == 0 && thread_interrupted(&x->threads[i])))
    {
      for (int i = 0; i < x->num_threads; ++i)
        thread_interrupt(&x->threads[i]);
    }

#pragma omp critical
    if (r && !rc) rc = r;
  }
  if (rc) defer_return(rc);

  return product_close(&x->product, x->num_threads);

defer:
  product_close(&x->product, x->num_threads);
  return rc;
}

void dcp_scan_interrupt(struct dcp_scan *x)
{
  for (int i = 0; i < x->num_threads; ++i)
    thread_interrupt(&x->threads[i]);
}

int dcp_scan_progress(struct dcp_scan const *x)
{
  return (100 * x->done_proteins) / x->database_reader.num_proteins;
}
