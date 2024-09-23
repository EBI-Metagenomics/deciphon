#include "aye.h"
#include "batch.h"
#include "fs.h"
#include "imm_rnd.h"
#include "scan.h"
#include "test_utils.h"

#define HMMFILE "massive.hmm"
#define DBFILE "massive.dcp"
#define PRODDIR "massive_product"
#define PORT 51300
#define NUM_THREADS 1
#define MULTI_HITS true
#define HMMER3_COMPAT false

static struct imm_rnd rnd = {0};

static char random_dna_nuclt(void) { return "ACGT"[imm_rnd_u64(&rnd) % 4]; }

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

int main(void)
{
  aye_begin();
  setup_database(1, 0.01, HMMFILE, DBFILE);
  fs_rmtree(PRODDIR);
  random_sequence_init();
  char name[256] = {};
  struct scan *scan = NULL;
  struct batch *batch = NULL;

  aye(batch = batch_new());
  aye(scan = scan_new());
  aye(scan_setup(scan, DBFILE, PORT, NUM_THREADS, true, false, NULL, NULL) == 0);
  for (int i = 0; i < 10000; ++i)
  {
    snprintf(name, sizeof(name), "name%d", i);
    aye(batch_add(batch, i, name, random_sequence_next()) == 0);
  }
  aye(scan_run(scan, batch, PRODDIR) == 0);
  aye(scan_progress(scan) == 100);
  aye(chksum(PRODDIR "/products.tsv") == 48347);

  batch_del(batch);
  scan_del(scan);
  fs_rmtree(PRODDIR);
  cleanup_database(DBFILE);
  return aye_end();
}
