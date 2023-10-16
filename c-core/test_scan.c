#include "deciphon/array_size.h"
#include "deciphon/fs.h"
#include "deciphon/press.h"
#include "deciphon/scan.h"
#include "deciphon/scan_params.h"
#include "deciphon/seq.h"
#include "imm/imm.h"
#include "vendor/minctest.h"

#define HMMFILE "minifam.hmm"
#define DBFILE "minifam.dcp"

static void test_scan(struct scan_params, long desired_chksum);
static void setup_minifam(void);
static void cleanup_minifam(void);

static struct scan_params params_list[] = {
    {1, false, false, false}, {1, false, false, true}, {1, false, true, false},
    {1, false, true, true},   {1, true, false, false}, {1, true, false, true},
    {1, true, true, false},   {1, true, true, true}};
static long chksum_list[] = {22415, 22415, 39438, 39438,
                             52212, 52212, 52212, 52212};

int main(void)
{
  imm_fmt_set_f32("%.5g");
  setup_minifam();
  for (size_t i = 0; i < array_size(params_list); ++i)
    test_scan(params_list[i], chksum_list[i]);
  cleanup_minifam();
  return lfails;
}

static bool next_seq(struct seq *, void *);

static long chksum(char const *filename)
{
  long chk = 0;
  eq_or_exit(dcp_fs_cksum(filename, &chk), 0);
  return chk;
}

static void test_scan(struct scan_params params, long desired_chksum)
{
  struct scan *scan = scan_new();
  ok_or_exit(scan);

  eq(scan_dial(scan, 51371), 0);
  eq(scan_setup(scan, params), 0);

  size_t idx = 0;
  dcp_fs_rmtree("prod_scan");
  eq(scan_run(scan, DBFILE, next_seq, &idx, "prod_scan"), 0);
  eq(chksum("prod_scan/products.tsv"), desired_chksum);
  eq(dcp_fs_rmtree("prod_scan"), 0);

  scan_del(scan);
}

static struct seq
{
  long id;
  char const *name;
  char const *data;
} seqs[] = {
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
     "TCTCAGGAAGTCGTTTCGAAGGTGGCGACTCGCAAAATTCCAATGGAGTCACAACGCGAGTCGACT"},
    {4, "Homoserine_dh-multi-consensus1",
     "GG"
     "CCTATCATTTCGACGCTCAAGGAGTCGCTGACAGGTGACCGTATTACTCGAATCGAAGGGATATTAAACG"
     "GCACCCTGAATTACATTCTCACTGAGATGGAGGAAGAGGGGGCTTCATTCTCTGAGGCGCTGAAGGAGGC"
     "ACAGGAATTGGGCTACGCGGAAGCGGATCCTACGGACGATGTGGAAGGGCTAGATGCTGCTAGAAAGCTG"
     "GCAATTCTAGCCAGATTGGCATTTGGGTTAGAGGTCGAGTTGGAGGACGTAGAGGTGGAAGGAATTGAAA"
     "AGCTGACTGCCGAAGATATTGAAGAAGCGAAGGAAGAGGGTAAAGTTTTAAAACTAGTGGCAAGCGCCGT"
     "CGAAGCCAGGGTCAAGCCTGAGCTGGTACCTAAGTCACATCCATTAGCCTCGGTAAAAGGCTCTGACAAC"
     "GCCGTGGCTGTAGAAACGGAACGGGTAGGCGAACTCGTAGTGCAGGGACCAGGGGCTGGCGCAGAGCCAA"
     "CCGCATCCGCTGTACTCGCTGACCTTCTC"
     "CATAGGG"
     "CCTATCATTTCGACGCTCAAGGAGTCGCTGACAGGTGACCGTATTACTCGAATCGAAGGGATATTAAACG"
     "GCACCCTGAATTACATTCTCACTGAGATGGAGGAAGAGGGGGCTTCATTCTCTGAGGCGCTGAAGGAGGC"
     "ACAGGAATTGGGCTACGCGGAAGCGGATCCTACGGACGATGTGGAAGGGCTAGATGCTGCTAGAAAGCTG"
     "GCAATTCTAGCCAGATTGGCATTTGGGTTAGAGGTCGAGTTGGAGGACGTAGAGGTGGAAGGAATTGAAA"
     "AGCTGACTGCCGAAGATATTGAAGAAGCGAAGGAAGAGGGTAAAGTTTTAAAACTAGTGGCAAGCGCCGT"
     "CGAAGCCAGGGTCAAGCCTGAGCTGGTACCTAAGTCACATCCATTAGCCTCGGTAAAAGGCTCTGACAAC"
     "GCCGTGGCTGTAGAAACGGAACGGGTAGGCGAACTCGTAGTGCAGGGACCAGGGGCTGGCGCAGAGCCAA"
     "CCGCATCCGCTGTACTCGCTGACCTTCTC"
     "TT"},
    {5, "Homoserine_dh-multi-consensus2",
     "CCTATCATTTCGACGCTCAAGGAGTCGCTGACAGGTGACCGTATTACTCGAATCGAAGGGATATTAAACG"
     "GCACCCTGAATTACATTCTCACTGAGATGGAGGAAGAGGGGGCTTCATTCTCTGAGGCGCTGAAGGAGGC"
     "ACAGGAATTGGGCTACGCGGAAGCGGATCCTACGGACGATGTGGAAGGGCTAGATGCTGCTAGAAAGCTG"
     "GCAATTCTAGCCAGATTGGCATTTGGGTTAGAGGTCGAGTTGGAGGACGTAGAGGTGGAAGGAATTGAAA"
     "AGCTGACTGCCGAAGATATTGAAGAAGCGAAGGAAGAGGGTAAAGTTTTAAAACTAGTGGCAAGCGCCGT"
     "CGAAGCCAGGGTCAAGCCTGAGCTGGTACCTAAGTCACATCCATTAGCCTCGGTAAAAGGCTCTGACAAC"
     "GCCGTGGCTGTAGAAACGGAACGGGTAGGCGAACTCGTAGTGCAGGGACCAGGGGCTGGCGCAGAGCCAA"
     "CCGCATCCGCTGTACTCGCTGACCTTCTC"
     "CCTATCATTTCGACGCTCAAGGAGTCGCTGACAGGTGACCGTATTACTCGAATCGAAGGGATATTAAACG"
     "GCACCCTGAATTACATTCTCACTGAGATGGAGGAAGAGGGGGCTTCATTCTCTGAGGCGCTGAAGGAGGC"
     "ACAGGAATTGGGCTACGCGGAAGCGGATCCTACGGACGATGTGGAAGGGCTAGATGCTGCTAGAAAGCTG"
     "GCAATTCTAGCCAGATTGGCATTTGGGTTAGAGGTCGAGTTGGAGGACGTAGAGGTGGAAGGAATTGAAA"
     "AGCTGACTGCCGAAGATATTGAAGAAGCGAAGGAAGAGGGTAAAGTTTTAAAACTAGTGGCAAGCGCCGT"
     "CGAAGCCAGGGTCAAGCCTGAGCTGGTACCTAAGTCACATCCATTAGCCTCGGTAAAAGGCTCTGACAAC"
     "GCCGTGGCTGTAGAAACGGAACGGGTAGGCGAACTCGTAGTGCAGGGACCAGGGGCTGGCGCAGAGCCAA"
     "CCGCATCCGCTGTACTCGCTGACCTTCTC"},
    {6, "Homoserine_dh-multi-consensus3",
     "GG"
     "CCTATCATTTCGACGCTCAAGGAGTCGCTGACAGGTGACCGTATTACTCGAATCGAAGGGATATTAAACG"
     "GCACCCTGAATTACATTCTCACTGAGATGGAGGAAGAGGGGGCTTCATTCTCTGAGGCGCTGAAGGAGGC"
     "ACAGGAATTGGGCTACGCGGAAGCGGATCCTTCGGACGATGTGGAAGGGCTAGATGCTGCTAGAAAGCTG"
     "GCAATTCTAGCCAGATTGGCATTTGGGTTAGAGGTCGAGTTGGAGGACGTAGCGGTGGAAGGAATTGAAA"
     "AGCTGACTGCCGAAGATATTGAAGAAGCGAAGGAAGAGGGTAAAGTTTTAAAACTAGTGGCAAGCGCCGT"
     "CGAAGCCAGGGTCAAGCCTGAGCTGGTACCTAAGTCACATCCATTAGCCTCGGTAAAAGGCTCTGACAAC"
     "GCCGTGGCTGTAGAAACGGAACGGGTAGGCGAACTCGTAGTGCAGGGACCAGGGGCTGGCGCAGAGCCAA"
     "CCGCATCCGCTGTACTCGCTGACCTTCTC"
     "CATTTAGGG"
     "CCTATCATTTCGACGCTCAAGGAGTCGCTGACAGGTGACCGTATTACTCGAATCGAAGGGATATTAAACG"
     "GCACCCTGAATTACATTCTCACTGAGATGGAGGAAGAGGGGGCTTCATTCTCTGAGGCGCTGAAGGAGGC"
     "ACAGGAATTGGGCTACGCGGAAGCGGATCCTACGGACGAAAAAAAAAAAAAAAAAGCTGCTAGAAAGCTG"
     "GCAATTCTAGCCAGATTGGCATTTGGGTTAGAGGTCGAGTTGGAGGACGTAGAGGTGGAAGGAATTGAAA"
     "AGCTGACTGCCGAAGATATTGAAGAAGCGAAGGAAGAGGGTAAAGTTTTAAAACTAGTGGCAAGCGCCGT"
     "CGAAGCCAGGGTCAAGCCTGAGCTGGTACCTAAGTCACATCCATTAGCCTCGGTAAAAGGCTCTGACAAC"
     "GCCGTGGCTGTAGAAACGGAACGGGTAGGCGAACTCGTAGTGCAGGGACCAGGGGCTGGCGCAGAGCCAA"
     "CCGCATCCGCTGTACTCGCTGACCTTCTC"
     "TT"},
    {7, "Homoserine_dh-multi-consensus3",
     "GG"
     "CCTATCATTTCGACGCTCAAGGAGTCGCTGACAGGTGACCGTATTACTCGAATCGAAGGGATATTAAACG"
     "GCACCCTGAATTACATTCTCACTGAGATGGAGGAAGAGGGGGCTTCATTCTCTGAGGCGCTGAAGGAGGC"
     "ACAGGAATTGGGCTACGCGGAAGCGGATCCTTCGGACGATGTGGAAGGGCTAGATGCTGCTAGAAAGCTG"
     "GCAATTCTAGCCAGATTGGCATTTGGGTTAGAGGTCGAGTTGGAGGACGTAGCGGTGGAAGGAATTGAAA"
     "AGCTGACTGCCGAAGATATTGAAGAAGCGAAGGAAGAGGGTAAAGTTTTAAAACTAGTGGCAAGCGCCGT"
     "CGAAGCCAGGGTCAAGCCTGAGCTGGTACCTAAGTCACATCCATTAGCCTCGGTAAAAGGCTCTGACAAC"
     "GCCGTGGCTGTAGAAACGGAACGGGTAGGCGAACTCGTAGTGCAGGGACCAGGGGCTGGCGCAGAGCCAA"
     "CCGCATCCGCTGTACTCGCTGACCTTCTC"
     "CATTTAGGG"
     "CCTATCATTTCGACGCTCAAGGAGTCGCTGACAGGTGACCGTATTACTCGAATCGAAGGGATATTAAACG"
     "CTGAATTACATTCTCACTGAGATGGAGGAAGAGGGGGCTTCATTCTCTGAGGCGCTGAAGGAGGC"
     "AATTGGGCTACGCGGAAGCGGATCCTACGGACGAAAAAAAGATGGAGAAAAAAAAAAGCTTAGAAAGCTG"
     "GCAATTCTAGCCAGATTGGCATTTGGGTTAGAGGTCGAGTTGGAGGACGTAGAGGTGGAAGGAATTGAAA"
     "AGCTGACTGCCGAAGATATTGAAGAAGCGAAGGAAGAGGGTAAAGTTTTAAAACTAGTGGCAAGCGCCGT"
     "CGAAGCCAGGGTCAAGCCTGAGCTGGTACCTAAGTCACATCCATTAGCCTCGGTAAAAGGCTCTGACAAC"
     "GCCGTGGCTGTAGAAACGGAACGGGTAGGCGAACTCGTAGTGCAGGGACCAGGGGCTGGCGCAGAGCCAA"
     "CCGCATCCGCTGTACTCGCTGACCTTCTC"
     "TT"},
    {8, "Homoserine_dh-multi-consensus1",
     "GG"
     "CCTATCATTTCGACGCTCAAGGAGTCGCTGACAGGTGACCGTATTACTCGAATCGAAGGGATATTAAACG"
     "GCACCCTGAATTACATTCTCACTGAGATGGAGGAAGAGGGGGCTTCATTCTCTGAGGCGCTGAAGGAGGC"
     "ACAGAATTGGGCTACGCGGAAGCGGATCCTACGGACGATGTGGAAGGGCTAGATGCTGCTAGAAAGCTG"
     "GCAATTCTAGCCAGATTGGCATTTGGGTTAGAGGTCGAGTTGGAGGACGTAGAGGTGGAAGGAATTGAAA"
     "AGCTGACTGCCGAAGATATTGAAGAAGCGAAGGAAGAGGGTAAAGTTTTAAAACTAGTGGCAAGCGCCGT"
     "CGAAGCCAGGGTCAAGCCTGAGCTGGTACCTAAGTCACATCCATTAGCCTCGGTAAAAGGCTCTGACAAC"
     "GCCGTGCTGTAGAAACGGAACGGGTAGGCGAACTCGTAGTGCAGGGACCAGGGGCTGGCGCAGAGCCAA"
     "CCGCATCCGCTGTACTCGCTGACCTTCTC"
     "CATAGG"
     "CCTATCATTTCGACGCTCAAGGAGTCGCTGACAGGTGACCGTATTACTCGAATCGAAGGGATATTAAACG"
     "GCACCCTGAATTACATTCTCACTGAGATGGAGGAAGAGGGGAAAAAAAAAAAAAAGGCGCTGAAGGAGGC"
     "ACAGGATTGGGCTACGCGGAAGCGGATCCTACGGACGATGTGGAAGGGCTAGATGCTGCTAGAAAGCTG"
     "GCAATTCTAGCCAGATTGGCATTTGGGTTAGAGGTCGAGTTGGAGGACGTAGAGGTGGAAGGAATTGAAA"
     "AGCTGACTGCCGAAGATATTGGCTTCGGCTTCGAAGAAGCGAAGGAAGAGGGTAAAGTTTTAAAACTAGT"
     "CGAAGCCAGGGTCAAGCCTGAGCTGGTACCTAAGTCACATCATTAGCCTCGGTAAAAGGCTCTGACAAC"
     "GCCGTGGCTGTAGAAACGGAACGGCCCCCCGAACTCGTAGTGCAGGGACCAGGGGCTGGCGCAGAGCCAA"
     "CCGCATCCGCTGTACTCGCTTTTTTTCTC"
     "TT"}};

static bool next_seq(struct seq *x, void *arg)
{
  size_t *i = arg;
  if (*i < array_size(seqs))
  {
    eq_or_exit(seq_setup(x, seqs[*i].id, seqs[*i].name, seqs[*i].data), 0);
    *i += 1;
    return true;
  }
  return false;
}

static void setup_minifam(void)
{
  struct press *press = press_new();

  eq_or_exit(press_setup(press, 1, 0.01), 0);
  eq_or_exit(press_open(press, HMMFILE, DBFILE), 0);

  eq_or_exit(press_nproteins(press), 3);
  int rc = 0;
  while (!press_end(press))
  {
    if ((rc = press_next(press))) break;
  }
  eq_or_exit(rc, 0);

  eq_or_exit(press_close(press), 0);
  press_del(press);
}

static void cleanup_minifam(void) { remove(DBFILE); }
