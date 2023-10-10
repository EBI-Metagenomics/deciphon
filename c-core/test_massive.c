#include "fs.h"
#include "imm/imm.h"
#include "press.h"
#include "scan.h"
#include "scan_params.h"
#include "test_seqit.h"
#include "vendor/minctest.h"

#define NUM_SEQS 10000

#define PORT 51372
#define HMMFILE "three.hmm"
#define DBFILE "three.dcp"
#define PRODDIR "prod_massive"
#define EPSILON 0.01
#define GENCODE 1
#define LRT_THRESHOLD -1000.
#define MULTI_HITS true
#define HMMER3_COMPAT false

static long chksum(char const *filename)
{
  long chk = 0;
  eq_or_exit(dcp_fs_cksum(filename, &chk), 0);
  return chk;
}

static bool next_seq(struct dcp_seq *x, void *seqit)
{
  struct seq const *seq = seqit_next((struct seqit *)seqit);
  if (!seq) return false;
  eq_or_exit(dcp_seq_setup(x, seq->id, seq->name, seq->data), 0);
  return true;
}

static void setup_database(void)
{
  struct dcp_press *press = dcp_press_new();

  eq_or_exit(dcp_press_setup(press, GENCODE, EPSILON), 0);
  eq_or_exit(dcp_press_open(press, HMMFILE, DBFILE), 0);

  eq_or_exit(dcp_press_nproteins(press), 1);
  int rc = 0;
  while (!dcp_press_end(press))
  {
    if ((rc = dcp_press_next(press))) break;
  }
  eq_or_exit(rc, 0);

  eq_or_exit(dcp_press_close(press), 0);
  dcp_press_del(press);
}

static void cleanup_database(void) { dcp_fs_rmfile(DBFILE); }

static void random_sequence_init(void);
static char const *random_sequence_next(void);

int main(void)
{
  setup_database();
  random_sequence_init();
  imm_fmt_set_f32("%.5g");

  struct dcp_scan *scan = dcp_scan_new();
  ok_or_exit(scan);
  struct dcp_scan_params params = {1, LRT_THRESHOLD, MULTI_HITS, HMMER3_COMPAT,
                                   true};
  eq_or_exit(dcp_scan_setup(scan, params), 0);

  struct seqit it = seqit_init(NUM_SEQS, &random_sequence_next);
  dcp_fs_rmtree(PRODDIR);
  eq(dcp_scan_run(scan, DBFILE, next_seq, &it, PRODDIR), 0);
  ok(chksum(PRODDIR "/products.tsv") == 55120 ||
     chksum(PRODDIR "/products.tsv") == 41793);
  eq(dcp_fs_rmtree(PRODDIR), 0);

  dcp_scan_del(scan);
  cleanup_database();
  return lfails;
}

static struct imm_rnd rnd = {0};

static inline char random_dna_nuclt(void)
{
  return "ACGT"[imm_rnd_u64(&rnd) % 4];
}

static void random_sequence_init(void) { rnd = imm_rnd(28911); }

static char const *random_sequence_next(void)
{
  static char seq[256] = {0};
  int size = 1 + imm_rnd_u64(&rnd) % (3 * 3 * 2 + 3);
  for (int j = 0; j < size; ++j)
    seq[j] = random_dna_nuclt();
  seq[size] = '\0';
  return seq;
}
