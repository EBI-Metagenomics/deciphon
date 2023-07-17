#include "deciphon/fs.h"
#include "deciphon/press.h"
#include "deciphon/scan.h"
#include "deciphon/scan_params.h"
#include "deciphon/seq.h"
#include "vendor/minctest.h"
#include <stdlib.h>

#include <sys/stat.h>

#define HMMFILE "minifam.hmm"
#define DBFILE "minifam.dcp"

static void test_scan1(void);
static void test_scan2(void);
static void test_scan3(void);

static void setup_minifam(void);
static void cleanup_minifam(void);

int main(void)
{
  setup_minifam();
  test_scan1();
  test_scan2();
  test_scan3();
  cleanup_minifam();
  return lfails;
}

struct test_seq
{
  long id;
  char const *name;
  char const *data;
};

static bool next_seq(struct dcp_seq *, void *);

static long fs_size(char const *filepath)
{
  struct stat st = {0};
  if (stat(filepath, &st)) return -1;
  return (long)st.st_size;
}

static void test_scan1(void)
{
  fprintf(stderr, "test_scan1\n");
  struct dcp_scan *scan = dcp_scan_new();
  ok_or_exit(scan);

  eq(dcp_scan_dial(scan, 51371), 0);
  struct dcp_scan_params params = dcp_scan_params(1, 10., true, false);
  eq(dcp_scan_setup(scan, params), 0);

  int idx = 0;
  eq(dcp_scan_run(scan, DBFILE, next_seq, &idx, "prod1"), 0);
  eq(fs_size("prod1/products.tsv"), 8646);
  eq(dcp_fs_rmtree("prod1"), 0);

  dcp_scan_del(scan);
}

static void test_scan2(void)
{
  fprintf(stderr, "test_scan2\n");
  struct dcp_scan *scan = dcp_scan_new();
  ok_or_exit(scan);

  eq(dcp_scan_dial(scan, 51371), 0);
  struct dcp_scan_params params = dcp_scan_params(2, 10., true, false);
  eq(dcp_scan_setup(scan, params), 0);

  int idx = 0;
  eq(dcp_scan_run(scan, DBFILE, next_seq, &idx, "prod2"), 0);
  eq(fs_size("prod2/products.tsv"), 8646);
  eq(dcp_fs_rmtree("prod2"), 0);

  dcp_scan_del(scan);
}

static void test_scan3(void)
{
  fprintf(stderr, "test_scan3\n");
  struct dcp_scan *scan = dcp_scan_new();

  eq(dcp_scan_dial(scan, 51371), 0);
  struct dcp_scan_params params = dcp_scan_params(2, 2., true, false);
  eq(dcp_scan_setup(scan, params), 0);

  int idx = 0;
  eq(dcp_scan_run(scan, DBFILE, next_seq, &idx, "prod3"), 0);
  eq(fs_size("prod3/products.tsv"), 8646);
  eq(dcp_fs_rmtree("prod3"), 0);

  dcp_scan_del(scan);
}

static bool next_seq(struct dcp_seq *x, void *arg)
{
  static struct test_seq seqs[] = {
      {1, "Homoserine_dh-consensus",
       "CCTATCATTTCGACGCTCAAGGAGTCGCTGACAGGTGACCGTATTACTCGAATCGAAGGGATATTAAACG"
       "GCACCCTGAATTACATTCTCACTGAGATGGAGGAAGAGGGGGCTTCATTCTCTGAGGCGCTGAAGGAGGC"
       "ACAGGAATTGGGCTACGCGGAAGCGGATCCTACGGACGATGTGGAAGGGCTAGATGCTGCTAGAAAGCTG"
       "GCAATTCTAGCCAGATTGGCATTTGGGTTAGAGGTCGAGTTGGAGGACGTAGAGGTGGAAGGAATTGAAA"
       "AGCTGACTGCCGAAGATATTGAAGAAGCGAAGGAAGAGGGTAAAGTTTTAAAACTAGTGGCAAGCGCCGT"
       "CGAAGCCAGGGTCAAGCCTGAGCTGGTACCTAAGTCACATCCATTAGCCTCGGTAAAAGGCTCTGACAAC"
       "GCCGTGGCTGTAGAAACGGAACGGGTAGGCGAACTCGTAGTGCAGGGACCAGGGGCTGGCGCAGAGCCAA"
       "CCGCATCCGCTGTACTCGCTGACCTTCTC"},
      {2, "AA_kinase-consensus",
       "AAACGTGTAGTTGTAAAGCTTGGGGGTAGTTCTCTGACAGATAAGGAAGAGGCATCACTCAGGCGTTTAG"
       "CTGAGCAGATTGCAGCATTAAAAGAGAGTGGCAATAAACTAGTGGTCGTGCATGGAGGCGGCAGCTTCAC"
       "TGATGGTCTGCTGGCATTGAAAAGTGGCCTGAGCTCGGGCGAATTAGCTGCGGGGTTGAGGAGCACGTTA"
       "GAAGAGGCCGGAGAAGTAGCGACGAGGGACGCCCTAGCTAGCTTAGGGGAACGGCTTGTTGCAGCGCTGC"
       "TGGCGGCGGGTCTCCCTGCTGTAGGACTCAGCGCCGCTGCGTTAGATGCGACGGAGGCGGGCCGGGATGA"
       "AGGCAGCGACGGGAACGTCGAGTCCGTGGACGCAGAAGCAATTGAGGAGTTGCTTGAGGCCGGGGTGGTC"
       "CCCGTCCTAACAGGATTTATCGGCTTAGACGAAGAAGGGGAACTGGGAAGGGGATCTTCTGACACCATCG"
       "CTGCGTTACTCGCTGAAGCTTTAGGCGCGGACAAACTCATAATACTGACCGACGTAGACGGCGTTTACGA"
       "TGCCGACCCTAAAAAGGTCCCAGACGCGAGGCTCTTGCCAGAGATAAGTGTGGACGAGGCCGAGGAAAGC"
       "GCCTCCGAATTAGCGACCGGTGGGATGAAGGTCAAACATCCAGCGGCTCTTGCTGCAGCTAGACGGGGGG"
       "GTATTCCGGTCGTGATAACGAAT"},
      {3, "23ISL-consensus",
       "CAGGGTCTGGATAACGCTAATCGTTCGCTAGTTCGCGCTACAAAAGCAGAAAGTTCAGATATACGGAAAG"
       "AGGTGACTAACGGCATCGCTAAAGGGCTGAAGCTAGACAGTCTGGAAACAGCTGCAGAGTCGAAGAACTG"
       "CTCAAGCGCACAGAAAGGCGGATCGCTAGCTTGGGCAACCAACTCCCAACCACAGCCTCTCCGTGAAAGT"
       "AAGCTTGAGCCATTGGAAGACTCCCCACGTAAGGCTTTAAAAACACCTGTGTTGCAAAAGACATCCAGTA"
       "CCATAACTTTACAAGCAGTCAAGGTTCAACCTGAACCCCGCGCTCCCGTCTCCGGGGCGCTGTCCCCGAG"
       "CGGGGAGGAACGCAAGCGCCCAGCTGCGTCTGCTCCCGCTACCTTACCGACACGACAGAGTGGTCTAGGT"
       "TCTCAGGAAGTCGTTTCGAAGGTGGCGACTCGCAAAATTCCAATGGAGTCACAACGCGAGTCGACT"}};

  int *i = (int *)arg;
  if (*i > 2) return false;
  eq_or_exit(dcp_seq_setup(x, seqs[*i].id, seqs[*i].name, seqs[*i].data), 0);
  *i += 1;
  return true;
}

static void setup_minifam(void)
{
  struct dcp_press *press = dcp_press_new();

  eq_or_exit(dcp_press_setup(press, 1, 0.01), 0);
  eq_or_exit(dcp_press_open(press, HMMFILE, DBFILE), 0);

  eq_or_exit(dcp_press_nproteins(press), 3);
  int rc = 0;
  while (!dcp_press_end(press))
  {
    if ((rc = dcp_press_next(press))) break;
  }
  eq_or_exit(rc, 0);

  eq_or_exit(dcp_press_close(press), 0);
  dcp_press_del(press);
}

static void cleanup_minifam(void) { remove(DBFILE); }
