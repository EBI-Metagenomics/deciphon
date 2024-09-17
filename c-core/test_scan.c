#include "array_size.h"
#include "aye.h"
#include "error.h"
#include "fs.h"
#include "params.h"
#include "scan.h"
#include "test_consensus.h"
#include "test_utils.h"

#define HMMFILE "minifam.hmm"
#define DBFILE "test_scan.dcp"
#define PRODDIR "test_scan_product"

static struct params params_list[] = {{51300, 1, false, false},
                                      {51300, 1, false, true},
                                      {51300, 1, true, false},
                                      {51300, 1, true, true}};
static long chksum_list[] = {6777, 5507, 42679, 17082};

static void test_invalid_sequence();
static void test_normal_scan(void);

int main(void)
{
  aye_begin();
  test_invalid_sequence();
  test_normal_scan();
  return aye_end();
}

static void test_invalid_sequence()
{
  setup_database(1, 0.01, HMMFILE, DBFILE);
  struct scan *scan = NULL;

  struct params params = {.port = 51300,
                          .num_threads = 1,
                          .multi_hits = true,
                          .hmmer3_compat = false};
  aye(scan = scan_new(params));
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

  for (size_t i = 0; i < array_size(params_list); ++i)
  {
    fs_rmtree(PRODDIR);
    aye(scan = scan_new(params_list[i]));
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
    aye(chksum(PRODDIR "/products.tsv") == chksum_list[i]);
    fs_rmtree(PRODDIR);
  }

  cleanup_database(DBFILE);
}
