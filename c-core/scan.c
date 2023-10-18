#include "scan.h"
#include "db_reader.h"
#include "defer_return.h"
#include "hmmer_dialer.h"
#include "product.h"
#include "protein_reader.h"
#include "queue.h"
#include "scan_thread.h"
#include "scan_thread_params.h"
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
    struct db_reader reader;
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
    scan_thread_init(x->threads + i);

  product_init(&x->product);
  sequence_queue_init(&x->sequences);

  db_reader_init(&x->db.reader);
  protein_reader_init(&x->db.protein);

  hmmer_dialer_init(&x->dialer);

  return x;
}

void scan_del(struct scan const *x)
{
  if (x)
  {
    struct scan *y = (struct scan *)x;
    hmmer_dialer_cleanup(&y->dialer);
    db_reader_close(&y->db.reader);
    sequence_queue_cleanup(&y->sequences);
    product_close(&y->product);
    for (int i = 0; i < x->params.num_threads; ++i)
      scan_thread_cleanup(y->threads + i);
    free(y);
  }
}

int scan_dial(struct scan *x, int port)
{
  return hmmer_dialer_setup(&x->dialer, port);
}

int scan_open(struct scan *x, char const *dbfile)
{
  int rc = 0;
  int num_threads = x->params.num_threads;

  if ((rc = db_reader_open(&x->db.reader, dbfile))) defer_return(rc);
  if ((rc = protein_reader_setup(&x->db.protein, &x->db.reader, num_threads)))
    defer_return(rc);
  sequence_queue_setup(&x->sequences, &x->db.reader.code.super);

defer:
  return rc;
}

int scan_close(struct scan *x)
{
  sequence_queue_cleanup(&x->sequences);
  return db_reader_close(&x->db.reader) ? DCP_EFCLOSE : 0;
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
    struct scan_thread_params params = {};
    scan_thread_params_init(&params, &x->params, &x->dialer, &x->db.protein,
                            product_thread(&x->product, i), i);
    if ((rc = scan_thread_setup(x->threads + i, params))) defer_return(rc);
  }

#pragma omp parallel for default(none) shared(x, num_threads, rc)
  for (int i = 0; i < num_threads; ++i)
  {
    int r = scan_thread_run(x->threads + i, &x->sequences);
#pragma omp critical
    if (r && !rc) rc = r;
  }

defer:
  for (int i = 0; i < num_threads; ++i)
    scan_thread_cleanup(x->threads + i);

  if (rc)
    product_close(&x->product);
  else
    rc = product_close(&x->product);

  return rc;
}
