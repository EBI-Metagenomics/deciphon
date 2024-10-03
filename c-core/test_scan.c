#include "array_size.h"
#include "aye.h"
#include "batch.h"
#include "deciphon.h"
#include "fs.h"
#include "test_consensus.h"
#include "test_utils.h"

#define HMMFILE "minifam.hmm"
#define DBFILE "scan.dcp"
#define PRODDIR "scan_product"
#define PORT 51300
#define NUM_THREADS 2

static bool multi_hits[] = {false, false, true, true};
static bool hmmer3_compat[] = {false, true, false, true};
static long normal_chksums[] = {6777, 5507, 42679, 17082};
static long reuse_chksums[] = {10266, 53532, 47384, 23833};

static void test_normal_scan(void)
{
  setup_database(1, 0.01, HMMFILE, DBFILE);
  struct dcp_scan *scan = NULL;
  struct dcp_batch *batch = NULL;

  aye(batch = dcp_batch_new());
  for (size_t j = 0; j < array_size(sequences); ++j)
  {
    long id = sequences[j].id;
    aye(dcp_batch_add(batch, id, sequences[j].name, sequences[j].data) == 0);
  }

  for (size_t i = 0; i < array_size(multi_hits); ++i)
  {
    fs_rmtree(PRODDIR);
    aye(scan = dcp_scan_new());
    aye(dcp_scan_setup(scan, DBFILE, PORT, NUM_THREADS, multi_hits[i],
                   hmmer3_compat[i], false, NULL, NULL) == 0);
    aye(dcp_scan_run(scan, batch, PRODDIR) == 0);
    aye(dcp_scan_progress(scan) == 100);
    dcp_scan_del(scan);
    aye(chksum(PRODDIR "/products.tsv") == normal_chksums[i]);
    fs_rmtree(PRODDIR);
  }

  dcp_batch_del(batch);
  cleanup_database(DBFILE);
}

static void test_reuse_scan(void)
{
  setup_database(1, 0.01, HMMFILE, DBFILE);
  struct dcp_scan *scan = NULL;
  struct dcp_batch *batch = NULL;
  aye(batch = dcp_batch_new());

  for (size_t i = 0; i < array_size(multi_hits); ++i)
  {
    fs_rmtree(PRODDIR);
    aye(scan = dcp_scan_new());
    aye(dcp_scan_setup(scan, DBFILE, PORT, NUM_THREADS, multi_hits[i],
                   hmmer3_compat[i], true, NULL, NULL) == 0);
    for (size_t j = 0; j < array_size(sequences); ++j)
    {
      long id = sequences[j].id;
      aye(dcp_batch_add(batch, id, sequences[j].name, sequences[j].data) == 0);
      aye(dcp_scan_run(scan, batch, PRODDIR) == 0);
      dcp_batch_reset(batch);
    }
    aye(dcp_scan_progress(scan) == 100);
    dcp_scan_del(scan);
    aye(chksum(PRODDIR "/products.tsv") == reuse_chksums[i]);
    fs_rmtree(PRODDIR);
  }

  dcp_batch_del(batch);
  cleanup_database(DBFILE);
}

int main(void)
{
  aye_begin();
  test_normal_scan();
  test_reuse_scan();
  return aye_end();
}
