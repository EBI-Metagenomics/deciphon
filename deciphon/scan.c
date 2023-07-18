#include "scan.h"
#include "defer_return.h"
#include "hmmer_dialer.h"
#include "prod_writer.h"
#include "scan_db.h"
#include "scan_thrd.h"
#include "seq.h"
#include "seq_iter.h"
#include <stdlib.h>

struct dcp_scan
{
  struct dcp_scan_thrd threads[DCP_NTHREADS_MAX];
  struct dcp_prod_writer prod_writer;

  struct dcp_scan_params params;

  struct dcp_scan_db db;
  struct dcp_seq_iter seqit;
  struct dcp_hmmer_dialer dialer;
};

struct dcp_scan *dcp_scan_new(void)
{
  struct dcp_scan *x = malloc(sizeof(*x));
  if (!x) return NULL;
  x->params = (struct dcp_scan_params){1, 0., true, false};
  dcp_scan_db_init(&x->db);
  dcp_prod_writer_init(&x->prod_writer);
  return x;
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

  if ((rc = dcp_scan_db_open(&x->db, dbfile, nthreads(x)))) defer_return(rc);

  dcp_seq_iter_init(&x->seqit, dcp_scan_code(&x->db), callb, userdata);

  if ((rc = dcp_prod_writer_open(&x->prod_writer, nthreads(x), product_dir)))
    defer_return(rc);

  struct dcp_scan_thrd_params params = {
      .reader = dcp_scan_db_reader(&x->db),
      .partition = 0,
      .prod_thrd = NULL,
      .dialer = &x->dialer,
      .lrt_threshold = x->params.lrt_threshold,
      .multi_hits = x->params.multi_hits,
      .hmmer3_compat = x->params.hmmer3_compat};

  for (int i = 0; i < nthreads(x); ++i)
  {
    params.partition = i;
    params.prod_thrd = dcp_prod_writer_thrd(&x->prod_writer, i);
    if ((rc = dcp_scan_thrd_setup(x->threads + i, params))) defer_return(rc);
  }

  int seq_idx = 0;
  while (dcp_seq_iter_next(&x->seqit) && !rc)
  {
    fprintf(stderr, "Scanning sequence %d\n", seq_idx++);
    struct dcp_seq *seq = dcp_seq_iter_get(&x->seqit);

#pragma omp parallel for default(none) shared(x, seq, rc)
    for (int i = 0; i < nthreads(x); ++i)
    {
      int r = dcp_scan_thrd_run(x->threads + i, seq);
#pragma omp critical
      if (r && !rc) rc = r;
    }
  }

defer:
  dcp_seq_iter_cleanup((struct dcp_seq_iter *)&x->seqit);
  dcp_scan_db_close(&x->db);
  for (int i = 0; i < nthreads(x); ++i)
    dcp_scan_thrd_cleanup(x->threads + i);

  if (rc)
    dcp_prod_writer_close(&x->prod_writer);
  else
    rc = dcp_prod_writer_close(&x->prod_writer);

  return rc;
}
