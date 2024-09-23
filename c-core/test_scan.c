#include "array_size.h"
#include "aye.h"
#include "batch.h"
#include "fs.h"
#include "scan.h"
#include "test_consensus.h"
#include "test_utils.h"

#define HMMFILE "minifam.hmm"
#define DBFILE "scan.dcp"
#define PRODDIR "scan_product"
#define PORT 51300
#define NUM_THREADS 1

static bool multi_hits[] = {false, false, true, true};
static bool hmmer3_compat[] = {false, true, false, true};
static long normal_chksums[] = {6777, 5507, 42679, 17082};
static long reuse_chksums[] = {10266, 53532, 47384, 23833};

static void test_normal_scan(void)
{
  setup_database(1, 0.01, HMMFILE, DBFILE);
  struct scan *scan = NULL;
  struct batch *batch = NULL;

  aye(batch = batch_new());
  for (size_t j = 0; j < array_size(sequences); ++j)
  {
    long id = sequences[j].id;
    aye(batch_add(batch, id, sequences[j].name, sequences[j].data) == 0);
  }

  for (size_t i = 0; i < array_size(multi_hits); ++i)
  {
    fs_rmtree(PRODDIR);
    aye(scan = scan_new());
    aye(scan_setup(scan, DBFILE, PORT, NUM_THREADS, multi_hits[i],
                   hmmer3_compat[i], NULL, NULL) == 0);
    aye(scan_run(scan, batch, PRODDIR) == 0);
    aye(scan_progress(scan) == 100);
    scan_del(scan);
    printf("normal: %ld\n", chksum(PRODDIR "/products.tsv"));
    aye(chksum(PRODDIR "/products.tsv") == normal_chksums[i]);
    fs_rmtree(PRODDIR);
  }

  batch_del(batch);
  cleanup_database(DBFILE);
}

static void test_reuse_scan(void)
{
  setup_database(1, 0.01, HMMFILE, DBFILE);
  struct scan *scan = NULL;
  struct batch *batch = NULL;
  aye(batch = batch_new());

  for (size_t i = 0; i < array_size(multi_hits); ++i)
  {
    fs_rmtree(PRODDIR);
    aye(scan = scan_new());
    aye(scan_setup(scan, DBFILE, PORT, NUM_THREADS, multi_hits[i],
                   hmmer3_compat[i], NULL, NULL) == 0);
    for (size_t j = 0; j < array_size(sequences); ++j)
    {
      long id = sequences[j].id;
      aye(batch_add(batch, id, sequences[j].name, sequences[j].data) == 0);
      aye(scan_run(scan, batch, PRODDIR) == 0);
      batch_reset(batch);
    }
    aye(scan_progress(scan) == 100);
    scan_del(scan);
    printf("reuse: %ld\n", chksum(PRODDIR "/products.tsv"));
    aye(chksum(PRODDIR "/products.tsv") == reuse_chksums[i]);
    fs_rmtree(PRODDIR);
  }

  batch_del(batch);
  cleanup_database(DBFILE);
}

int main(void)
{
  aye_begin();
  test_normal_scan();
  test_reuse_scan();
  return aye_end();
}
