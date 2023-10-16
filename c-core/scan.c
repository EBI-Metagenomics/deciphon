#include "scan.h"
#include "db_reader.h"
#include "defer_return.h"
#include "hmmer_dialer.h"
#include "product.h"
#include "protein_reader.h"
#include "queue.h"
#include "scan_thread.h"
#include "seq.h"
#include "seq_iter.h"
#include <stdlib.h>

struct scan
{
  struct scan_thread threads[DCP_NTHREADS_MAX];
  struct product prod_writer;

  struct scan_params params;

  struct
  {
    FILE *fp;
    struct db_reader reader;
    struct protein_reader protein;
  } db;

  struct dcp_seq_iter seqit;
  struct hmmer_dialer dialer;
};

struct scan *scan_new(void)
{
  struct scan *x = malloc(sizeof(*x));
  if (!x) return NULL;
  x->params = (struct scan_params){1, true, false, false};
  x->db.fp = NULL;
  db_reader_init(&x->db.reader);
  protein_reader_init(&x->db.protein);
  product_init(&x->prod_writer);
  return x;
}

static void seqs_cleanup(struct queue *seqs)
{
  struct iter iter = queue_iter(seqs);
  struct dcp_seq *tmp = NULL;
  struct dcp_seq *seq = NULL;
  iter_for_each_entry_safe(seq, tmp, &iter, node)
  {
    free((void *)seq->name);
    dcp_seq_cleanup(seq);
    free(seq);
  }
}

int scan_dial(struct scan *x, int port)
{
  return hmmer_dialer_init(&x->dialer, port);
}

int scan_setup(struct scan *x, struct scan_params params)
{
  if (params.num_threads > DCP_NTHREADS_MAX) return DCP_EMANYTHREADS;
  x->params = params;
  return 0;
}

void scan_del(struct scan const *x)
{
  if (x)
  {
    hmmer_dialer_cleanup((struct hmmer_dialer *)&x->dialer);
    free((void *)x);
  }
}

static inline int nthreads(struct scan *x) { return x->params.num_threads; }

int scan_run(struct scan *x, char const *dbfile, dcp_seq_next_fn *callb,
                 void *userdata, char const *product_dir)
{
  int rc = 0;

  for (int i = 0; i < nthreads(x); ++i)
    scan_thread_init(x->threads + i);

  if (!(x->db.fp = fopen(dbfile, "rb"))) defer_return(DCP_EOPENDB);
  if ((rc = db_reader_open(&x->db.reader, x->db.fp))) defer_return(rc);
  if ((rc = protein_reader_setup(&x->db.protein, &x->db.reader, nthreads(x))))
    defer_return(rc);

  dcp_seq_iter_init(&x->seqit, &x->db.reader.code.super, callb, userdata);
  struct queue seqs = QUEUE_INIT(seqs);
  while (dcp_seq_iter_next(&x->seqit))
  {
    struct dcp_seq *tmp = dcp_seq_iter_get(&x->seqit);
    struct dcp_seq *seq = dcp_seq_clone(tmp);
    dcp_seq_cleanup(tmp);
    if (!seq) defer_return(DCP_ENOMEM);
    queue_put(&seqs, &seq->node);
  }

  if ((rc = product_open(&x->prod_writer, nthreads(x), product_dir)))
    defer_return(rc);

  struct scan_thread_params params = {.reader = &x->db.protein,
                                      .partition = 0,
                                      .prod_thrd = NULL,
                                      .dialer = &x->dialer,
                                      .multi_hits = x->params.multi_hits,
                                      .hmmer3_compat = x->params.hmmer3_compat,
                                      .disable_hmmer = x->params.disable_hmmer};

  for (int i = 0; i < nthreads(x); ++i)
  {
    params.partition = i;
    params.prod_thrd = product_thread(&x->prod_writer, i);
    if ((rc = scan_thread_setup(x->threads + i, params))) defer_return(rc);
  }

#pragma omp parallel for default(none) shared(x, seqs, rc)
  for (int i = 0; i < nthreads(x); ++i)
  {
    int r = scan_thread_run(x->threads + i, &seqs);
#pragma omp critical
    if (r && !rc) rc = r;
  }

defer:
  seqs_cleanup(&seqs);
  dcp_seq_iter_cleanup((struct dcp_seq_iter *)&x->seqit);
  db_reader_close(&x->db.reader);
  if (x->db.fp)
  {
    fclose(x->db.fp);
    x->db.fp = NULL;
  }
  for (int i = 0; i < nthreads(x); ++i)
    scan_thread_cleanup(x->threads + i);

  if (rc)
    product_close(&x->prod_writer);
  else
    rc = product_close(&x->prod_writer);

  return rc;
}
