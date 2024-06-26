#include "array_size.h"
#include "error.h"
#include "fs.h"
#include "params.h"
#include "scan.h"
#include "test_consensus.h"
#include "test_utils.h"
#include "vendor/minctest.h"

#define HMMFILE "minifam.hmm"
#define DBFILE "test_scan.dcp"
#define PRODDIR "test_scan"

static struct params params_list[] = {
    {1, false, false}, {1, false, false}, {1, false, true}, {1, false, true},
    {1, true, false},  {1, true, false},  {1, true, true},  {1, true, true}};
static bool dial_list[] = {true, false, true, false, true, false, true, false};
static long chksum_list[] = {40213, 40213, 24863, 24863,
                             24535, 24535, 43482, 43482};

static void test_invalid_sequence();
static void test_normal_scan(void);

int main(void)
{
  test_invalid_sequence();
  test_normal_scan();
  return lfails;
}

static void test_invalid_sequence()
{
  setup_database(1, 0.01, HMMFILE, DBFILE);
  struct scan *scan = NULL;

  struct params params = {
      .num_threads = 1, .multi_hits = true, .hmmer3_compat = false};
  ok(scan = scan_new(params));
  eq(scan_open(scan, DBFILE), 0);
  long id = 1;
  eq(scan_add(scan, id, "damanged-dna", "ACGTMNZ"), DCP_ESEQABC);
  eq(scan_add(scan, id, "rna", "ACGU"), DCP_EDBDNASEQRNA);
  eq(scan_close(scan), 0);
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
    ok(scan = scan_new(params_list[i]));
    eq(scan_open(scan, DBFILE), 0);
    if (dial_list[i]) eq(scan_dial(scan, 51371), 0);
    for (size_t j = 0; j < array_size(sequences); ++j)
    {
      long id = sequences[j].id;
      eq(scan_add(scan, id, sequences[j].name, sequences[j].data), 0);
    }
    eq(scan_run(scan, PRODDIR, NULL, NULL), 0);
    eq(scan_progress(scan), 100);
    eq(scan_close(scan), 0);
    scan_del(scan);
    eq(chksum(PRODDIR "/products.tsv"), chksum_list[i]);
    fs_rmtree(PRODDIR);
  }

  cleanup_database(DBFILE);
}
