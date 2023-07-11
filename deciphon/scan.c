#include "scan.h"
#include "array_size_field.h"
#include "defer_return.h"
#include "hmmer_dialer.h"
#include "prod_writer.h"
#include "rc.h"
#include "scan_db.h"
#include "scan_thrd.h"
#include "seq.h"
#include "seqiter.h"
#include "size.h"
#include <stdlib.h>
#include <string.h>

struct dcp_scan
{
  int nthreads;
  struct scan_thrd threads[DCP_NTHREADS_MAX];
  struct prod_writer prod_writer;

  double lrt_threshold;
  bool multi_hits;
  bool hmmer3_compat;

  struct scan_db db;
  struct seqiter seqit;
  struct hmmer_dialer dialer;
};

struct dcp_scan *dcp_scan_new(int port)
{
  struct dcp_scan *x = malloc(sizeof(*x));
  x->nthreads = 1;
  x->lrt_threshold = 10.;
  x->multi_hits = true;
  x->hmmer3_compat = false;
  scan_db_init(&x->db);
  dcp_seqiter_init(&x->seqit);
  prod_writer_init(&x->prod_writer);
  if (dcp_hmmer_dialer_init(&x->dialer, port))
  {
    free(x);
    return NULL;
  }
  return x;
}

void dcp_scan_del(struct dcp_scan const *x)
{
  if (x)
  {
    dcp_hmmer_dialer_cleanup((struct hmmer_dialer *)&x->dialer);
    free((void *)x);
  }
}

int dcp_scan_set_nthreads(struct dcp_scan *x, int nthreads)
{
  if (nthreads > DCP_NTHREADS_MAX) return DCP_EMANYTHREADS;
  x->nthreads = nthreads;
  return 0;
}

void dcp_scan_set_lrt_threshold(struct dcp_scan *x, double lrt)
{
  x->lrt_threshold = lrt;
}

void dcp_scan_set_multi_hits(struct dcp_scan *x, bool multihits)
{
  x->multi_hits = multihits;
}

void dcp_scan_set_hmmer3_compat(struct dcp_scan *x, bool h3compat)
{
  x->hmmer3_compat = h3compat;
}

int dcp_scan_set_proteins(struct dcp_scan *x, char const *dbfile)
{
  return scan_db_set_filename(&x->db, dbfile);
}

void dcp_scan_set_sequences(struct dcp_scan *x, dcp_seq_next_fn *callb,
                            void *arg)
{
  dcp_seqiter_set_callback(&x->seqit, callb, arg);
}

int dcp_scan_run(struct dcp_scan *x, char const *name)
{
  int rc = 0;
  for (int i = 0; i < x->nthreads; ++i)
    memset(&x->threads[i], 0, sizeof(x->threads[i]));

  if ((rc = scan_db_open(&x->db, x->nthreads))) defer_return(rc);

  if ((rc = prod_writer_open(&x->prod_writer, x->nthreads, name)))
    defer_return(rc);

  for (int i = 0; i < x->nthreads; ++i)
  {
    struct scan_thrd *t = x->threads + i;
    struct protein_reader *r = scan_db_reader(&x->db);
    struct prod_writer_thrd *wt = prod_writer_thrd(&x->prod_writer, i);
    if ((rc = scan_thrd_init(t, r, i, wt, &x->dialer))) defer_return(rc);
    scan_thrd_set_lrt_threshold(t, x->lrt_threshold);
    scan_thrd_set_multi_hits(t, x->multi_hits);
    scan_thrd_set_hmmer3_compat(t, x->hmmer3_compat);
  }

  int seq_idx = 0;
  while (dcp_seqiter_next(&x->seqit) && !rc)
  {
    fprintf(stderr, "Scanning sequence %d\n", seq_idx++);
    struct dcp_seq const *y = dcp_seqiter_get(&x->seqit);
    struct iseq seq = iseq_init(y->id, y->name, y->data, scan_db_abc(&x->db));

#pragma omp parallel for default(none) shared(x, seq, rc)
    for (int i = 0; i < x->nthreads; ++i)
    {
      int r = scan_thrd_run(x->threads + i, &seq);
#pragma omp critical
      if (r && !rc) rc = r;
    }
  }
  if (rc) defer_return(rc);

  scan_db_close(&x->db);
  for (int i = 0; i < x->nthreads; ++i)
    scan_thrd_cleanup(x->threads + i);
  return prod_writer_close(&x->prod_writer);

defer:
  prod_writer_close(&x->prod_writer);
  scan_db_close(&x->db);
  for (int i = 0; i < x->nthreads; ++i)
    scan_thrd_cleanup(x->threads + i);
  return rc;
}
