#include "scan.h"
#include "db_reader.h"
#include "defer_return.h"
#include "hmmer_dialer.h"
#include "prod_writer.h"
#include "protein_reader.h"
#include "queue.h"
#include "scan_thrd.h"
#include "seq.h"
#include "seq_iter.h"
#include <stdlib.h>

struct dcp_scan
{
  struct dcp_scan_thrd threads[DCP_NTHREADS_MAX];
  struct dcp_prod_writer prod_writer;

  struct dcp_scan_params params;

  struct
  {
    FILE *fp;
    struct dcp_db_reader reader;
    struct dcp_protein_reader protein;
  } db;

  struct dcp_seq_iter seqit;
  struct dcp_hmmer_dialer dialer;
};

struct dcp_scan *dcp_scan_new(void)
{
  struct dcp_scan *x = malloc(sizeof(*x));
  if (!x) return NULL;
  x->params = (struct dcp_scan_params){1, 0., true, false, false};
  x->db.fp = NULL;
  dcp_db_reader_init(&x->db.reader);
  dcp_protein_reader_init(&x->db.protein);
  dcp_prod_writer_init(&x->prod_writer);
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

int dcp_scan_dial(struct dcp_scan *x, int port)
{
  return dcp_hmmer_dialer_init(&x->dialer, port);
}

int dcp_scan_setup(struct dcp_scan *x, struct dcp_scan_params params)
{
  if (params.num_threads > DCP_NTHREADS_MAX) return DCP_EMANYTHREADS;
  x->params = params;
  return 0;
}

void dcp_scan_del(struct dcp_scan const *x)
{
  if (x)
  {
    dcp_hmmer_dialer_cleanup((struct dcp_hmmer_dialer *)&x->dialer);
    free((void *)x);
  }
}

static inline int nthreads(struct dcp_scan *x) { return x->params.num_threads; }

int dcp_scan_run(struct dcp_scan *x, char const *dbfile, dcp_seq_next_fn *callb,
                 void *userdata, char const *product_dir)
{
  int rc = 0;

  for (int i = 0; i < nthreads(x); ++i)
    dcp_scan_thrd_init(x->threads + i);

  if (!(x->db.fp = fopen(dbfile, "rb"))) defer_return(DCP_EOPENDB);
  if ((rc = dcp_db_reader_open(&x->db.reader, x->db.fp))) defer_return(rc);
  if ((rc = dcp_protein_reader_setup(&x->db.protein, &x->db.reader,
                                     nthreads(x))))
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

  if ((rc = dcp_prod_writer_open(&x->prod_writer, nthreads(x), product_dir)))
    defer_return(rc);

  struct dcp_scan_thrd_params params = {
      .reader = &x->db.protein,
      .partition = 0,
      .prod_thrd = NULL,
      .dialer = &x->dialer,
      .lrt_threshold = x->params.lrt_threshold,
      .multi_hits = x->params.multi_hits,
      .hmmer3_compat = x->params.hmmer3_compat,
      .disable_hmmer = x->params.disable_hmmer};

  for (int i = 0; i < nthreads(x); ++i)
  {
    params.partition = i;
    params.prod_thrd = dcp_prod_writer_thrd(&x->prod_writer, i);
    if ((rc = dcp_scan_thrd_setup(x->threads + i, params))) defer_return(rc);
  }

#pragma omp parallel for default(none) shared(x, seqs, rc)
  for (int i = 0; i < nthreads(x); ++i)
  {
    int r = dcp_scan_thrd_run(x->threads + i, &seqs);
#pragma omp critical
    if (r && !rc) rc = r;
  }

defer:
  seqs_cleanup(&seqs);
  dcp_seq_iter_cleanup((struct dcp_seq_iter *)&x->seqit);
  dcp_db_reader_close(&x->db.reader);
  if (x->db.fp)
  {
    fclose(x->db.fp);
    x->db.fp = NULL;
  }
  for (int i = 0; i < nthreads(x); ++i)
    dcp_scan_thrd_cleanup(x->threads + i);

  if (rc)
    dcp_prod_writer_close(&x->prod_writer);
  else
    rc = dcp_prod_writer_close(&x->prod_writer);

  return rc;
}
