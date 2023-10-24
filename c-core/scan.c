#include "scan.h"
#include "database_reader.h"
#include "defer_return.h"
#include "hmmer_dialer.h"
#include "product.h"
#include "protein_reader.h"
#include "queue.h"
#include "sequence.h"
#include "sequence_queue.h"
#include "thread.h"
#include "thread_params.h"
#include <stdlib.h>

struct scan
{
  struct scan_params params;
  struct thread threads[DCP_NTHREADS_MAX];

  struct product product;
  struct sequence_queue sequences;

  struct
  {
    struct database_reader reader;
    struct protein_reader protein;
  } db;

  struct hmmer_dialer dialer;
};

struct scan *scan_new(struct scan_params params)
{
  struct scan *x = malloc(sizeof(*x));
  if (!x) return NULL;

  x->params = params;
  for (int i = 0; i < x->params.num_threads; ++i)
    thread_init(x->threads + i);

  product_init(&x->product);
  sequence_queue_init(&x->sequences);

  database_reader_init(&x->db.reader);
  protein_reader_init(&x->db.protein);

  hmmer_dialer_init(&x->dialer);

  return x;
}

void scan_del(struct scan const *scan)
{
  struct scan *x = (struct scan *)scan;
  if (x)
  {
    hmmer_dialer_cleanup(&x->dialer);
    database_reader_close(&x->db.reader);
    sequence_queue_cleanup(&x->sequences);
    product_close(&x->product);
    for (int i = 0; i < x->params.num_threads; ++i)
      thread_cleanup(x->threads + i);
    free(x);
  }
}

int scan_dial(struct scan *x, int port)
{
  return hmmer_dialer_setup(&x->dialer, port);
}

int scan_open(struct scan *x, char const *dbfile)
{
  int rc = 0;
  int n = x->params.num_threads;

  if ((rc = database_reader_open(&x->db.reader, dbfile))) return rc;
  if ((rc = protein_reader_setup(&x->db.protein, &x->db.reader, n))) return rc;
  sequence_queue_setup(&x->sequences, &x->db.reader.code.super);

  return rc;
}

int scan_close(struct scan *x)
{
  sequence_queue_cleanup(&x->sequences);
  return database_reader_close(&x->db.reader) ? DCP_EFCLOSE : 0;
}

int scan_add(struct scan *x, long id, char const *name, char const *data)
{
  return sequence_queue_put(&x->sequences, id, name, data);
}

int scan_run(struct scan *x, char const *product_dir)
{
  int rc = 0;
  int num_threads = x->params.num_threads;

  if ((rc = product_open(&x->product, num_threads, product_dir)))
    defer_return(rc);

  for (int i = 0; i < num_threads; ++i)
  {
    struct thread_params params = {&x->db.protein,
                                   i,
                                   product_thread(&x->product, i),
                                   &x->dialer,
                                   x->params.multi_hits,
                                   x->params.hmmer3_compat};
    if ((rc = thread_setup(x->threads + i, params))) defer_return(rc);
  }

#pragma omp parallel for default(none) shared(x, num_threads, rc)
  for (int i = 0; i < num_threads; ++i)
  {
    int r = thread_run(x->threads + i, &x->sequences);
#pragma omp critical
    if (r && !rc) rc = r;
  }

defer:
  for (int i = 0; i < num_threads; ++i)
    thread_cleanup(x->threads + i);

  int product_rc = product_close(&x->product);
  return rc ? rc : product_rc;
}
