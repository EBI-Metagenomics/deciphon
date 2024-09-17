#include "array_size.h"
#include "aye.h"
#include "error.h"
#include "fs.h"
#include "scan.h"
#include "test_consensus.h"
#include "test_utils.h"

#define HMMFILE "minifam.hmm"
#define DBFILE "test_scan.dcp"
#define PRODDIR "test_scan_product"
#define PORT 51300
#define NUM_THREADS 1

static bool multi_hits[] = {false, false, true, true};
static bool hmmer3_compat[] = {false, true, false, true};
static long chksums[] = {6777, 5507, 42679, 17082};

static void test_invalid_sequence()
{
  setup_database(1, 0.01, HMMFILE, DBFILE);
  struct scan *scan = NULL;

  aye(scan = scan_new());
  aye(scan_setup(scan, PORT, NUM_THREADS, true, false) == 0);
  aye(scan_open(scan, DBFILE) == 0);
  long id = 1;
  aye(scan_add(scan, id, "damanged-dna", "ACGTMNZ") == DCP_ESEQABC);
  aye(scan_add(scan, id, "rna", "ACGU") == DCP_EDBDNASEQRNA);
  aye(scan_close(scan) == 0);
  scan_del(scan);

  cleanup_database(DBFILE);
}

static void test_normal_scan(void)
{
  setup_database(1, 0.01, HMMFILE, DBFILE);
  struct scan *scan = NULL;

  for (size_t i = 0; i < array_size(multi_hits); ++i)
  {
    fs_rmtree(PRODDIR);
    aye(scan = scan_new());
    aye(scan_setup(scan, PORT, NUM_THREADS, multi_hits[i], hmmer3_compat[i]) == 0);
    aye(scan_open(scan, DBFILE) == 0);
    for (size_t j = 0; j < array_size(sequences); ++j)
    {
      long id = sequences[j].id;
      aye(scan_add(scan, id, sequences[j].name, sequences[j].data) == 0);
    }
    aye(scan_run(scan, PRODDIR, NULL, NULL) == 0);
    aye(scan_progress(scan) == 100);
    aye(scan_close(scan) == 0);
    scan_del(scan);
    aye(chksum(PRODDIR "/products.tsv") == chksums[i]);
    fs_rmtree(PRODDIR);
  }

  cleanup_database(DBFILE);
}

int main(void)
{
  aye_begin();
  test_invalid_sequence();
  test_normal_scan();
  return aye_end();
}
