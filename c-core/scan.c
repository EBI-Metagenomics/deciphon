#include "scan.h"
#include "db_reader.h"
#include "defer_return.h"
#include "hmmer_dialer.h"
#include "product.h"
#include "protein_reader.h"
#include "queue.h"
#include "scan_thread.h"
#include "sequence.h"
#include "sequence_queue.h"
#include <stdlib.h>

struct scan
{
  struct scan_params params;
  struct scan_thread threads[DCP_NTHREADS_MAX];

  struct product product;
  struct sequence_queue sequences;

  struct
  {
    FILE *fp;
    struct db_reader reader;
    struct protein_reader protein;
  } db;

  struct hmmer_dialer dialer;
};

struct scan *scan_new(void)
{
  struct scan *x = malloc(sizeof(*x));
  if (!x) return NULL;

  x->params = SCAN_PARAMS_DEFAULT();
  for (int i = 0; i < x->params.num_threads; ++i)
    scan_thread_init(x->threads + i);

  product_init(&x->product);
  sequence_queue_init(&x->sequences, NULL);

  x->db.fp = NULL;
  db_reader_init(&x->db.reader);
  protein_reader_init(&x->db.protein);

  return x;
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
    sequence_queue_cleanup((struct sequence_queue *)&x->sequences);
    free((void *)x);
  }
}

static inline int nthreads(struct scan *x) { return x->params.num_threads; }

int scan_open(struct scan *x, char const *dbfile)
{
  int rc = 0;

  if (!(x->db.fp = fopen(dbfile, "rb"))) defer_return(DCP_EOPENDB);
  if ((rc = db_reader_open(&x->db.reader, x->db.fp))) defer_return(rc);
  if ((rc = protein_reader_setup(&x->db.protein, &x->db.reader, nthreads(x))))
    defer_return(rc);

  sequence_queue_init(&x->sequences, &x->db.reader.code.super);

defer:
  return rc;
}

int scan_add(struct scan *x, long id, char const *name, char const *data)
{
  return sequence_queue_add(&x->sequences, id, name, data);
}

int scan_run(struct scan *x, char const *product_dir)
{
  int rc = 0;

  seq_iter_init(&x->seqit, &x->db.reader.code.super, callb, userdata);
  struct queue seqs = QUEUE_INIT(seqs);
  while (seq_iter_next(&x->seqit))
  {
    struct sequence *tmp = seq_iter_get(&x->seqit);
    struct sequence *seq = sequence_clone(tmp);
    sequence_cleanup(tmp);
    if (!seq) defer_return(DCP_ENOMEM);
    queue_put(&seqs, &seq->node);
  }

  if ((rc = product_open(&x->product, nthreads(x), product_dir)))
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
    params.prod_thrd = product_thread(&x->product, i);
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
  seq_iter_cleanup((struct seq_iter *)&x->seqit);
  db_reader_close(&x->db.reader);
  if (x->db.fp)
  {
    fclose(x->db.fp);
    x->db.fp = NULL;
  }
  for (int i = 0; i < nthreads(x); ++i)
    scan_thread_cleanup(x->threads + i);

  if (rc)
    product_close(&x->product);
  else
    rc = product_close(&x->product);

  return rc;
}

int scan_close(struct scan *x)
{
  db_reader_close(&x->db.reader);
  if (x->db.fp)
  {
    fclose(x->db.fp);
    x->db.fp = NULL;
  }
  for (int i = 0; i < nthreads(x); ++i)
    scan_thread_cleanup(x->threads + i);

  return product_close(&x->product);
}
