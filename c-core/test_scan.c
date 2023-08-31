#include "deciphon/fs.h"
#include "deciphon/press.h"
#include "deciphon/scan.h"
#include "deciphon/scan_params.h"
#include "deciphon/seq.h"
#include "imm/imm.h"
#include "vendor/minctest.h"

#include <sys/stat.h>

#define HMMFILE "minifam.hmm"
#define DBFILE "minifam.dcp"

static void test_scan1(void);
static void test_scan2(void);
static void test_scan3(void);
static void test_scan4(void);

static void setup_minifam(void);
static void cleanup_minifam(void);

static void combi_seq_init(void);
static bool combi_seq_next(struct dcp_seq *, void *);

int main(void)
{
  setup_minifam();
  test_scan1();
  test_scan2();
  test_scan3();
  test_scan4();
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

static void test_scan1(void)
{
  struct dcp_scan *scan = dcp_scan_new();
  ok_or_exit(scan);

  eq(dcp_scan_dial(scan, 51371), 0);
  struct dcp_scan_params params = {1, 10., true, false};
  eq(dcp_scan_setup(scan, params), 0);

  int idx = 0;
  eq(dcp_scan_run(scan, DBFILE, next_seq, &idx, "prod1"), 0);
  long chk = 0;
  eq_or_exit(dcp_fs_cksum("prod1/products.tsv", &chk), 0);
  ok(chk == 2817 || chk == 17890);
  eq(dcp_fs_rmtree("prod1"), 0);

  dcp_scan_del(scan);
}

static void test_scan2(void)
{
  struct dcp_scan *scan = dcp_scan_new();
  ok_or_exit(scan);

  eq(dcp_scan_dial(scan, 51371), 0);
  struct dcp_scan_params params = {2, 10., true, false};
  eq(dcp_scan_setup(scan, params), 0);

  int idx = 0;
  eq(dcp_scan_run(scan, DBFILE, next_seq, &idx, "prod2"), 0);
  long chk = 0;
  eq_or_exit(dcp_fs_cksum("prod2/products.tsv", &chk), 0);
  ok(chk == 2817 || chk == 17890);
  eq(dcp_fs_rmtree("prod2"), 0);

  dcp_scan_del(scan);
}

static void test_scan3(void)
{
  struct dcp_scan *scan = dcp_scan_new();

  eq(dcp_scan_dial(scan, 51371), 0);
  struct dcp_scan_params params = {2, 2., true, false};
  eq(dcp_scan_setup(scan, params), 0);

  int idx = 0;
  eq(dcp_scan_run(scan, DBFILE, next_seq, &idx, "prod3"), 0);
  long chk = 0;
  eq_or_exit(dcp_fs_cksum("prod3/products.tsv", &chk), 0);
  ok(chk == 2817 || chk == 17890);
  eq(dcp_fs_rmtree("prod3"), 0);

  dcp_scan_del(scan);
}

static void test_scan4(void)
{
  struct dcp_scan *scan = dcp_scan_new();

  eq(dcp_scan_dial(scan, 51371), 0);
  struct dcp_scan_params params = {1, 0., true, false};
  eq(dcp_scan_setup(scan, params), 0);

  combi_seq_init();
  eq(dcp_scan_run(scan, DBFILE, combi_seq_next, NULL, "prod4"), 0);
  long chk = 0;
  eq_or_exit(dcp_fs_cksum("prod4/products.tsv", &chk), 0);
  printf("chk: %ld\n", chk);
  ok(chk == 57189);
  eq(dcp_fs_rmtree("prod4"), 0);

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

static struct combi_seq
{
  unsigned nsymbols;
  char const *symbols;
  struct test_seq seq;

  unsigned seqlen;

  struct imm_cartes iter;
} combi = {0};

static void combi_seq_init(void)
{
  combi.nsymbols = imm_abc_size(&imm_dna_iupac.super.super);
  combi.symbols = imm_abc_symbols(&imm_dna_iupac.super.super);
  combi.seq.id = 0;
  combi.seq.name = "name";
  combi.seqlen = 1;
  imm_cartes_init(&combi.iter, combi.symbols, combi.nsymbols, combi.seqlen);
  imm_cartes_setup(&combi.iter, combi.seqlen);
}

static bool combi_seq_next(struct dcp_seq *x, void *arg)
{
  (void)arg;
  if ((combi.seq.data = imm_cartes_next(&combi.iter)))
  {
    eq_or_exit(dcp_seq_setup(x, combi.seq.id, combi.seq.name, combi.seq.data),
               0);
    return true;
  }

  imm_cartes_cleanup(&combi.iter);
  if (combi.seqlen >= 5) return false;

  combi.seqlen += 1;
  imm_cartes_init(&combi.iter, combi.symbols, combi.nsymbols, combi.seqlen);
  imm_cartes_setup(&combi.iter, combi.seqlen);

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
