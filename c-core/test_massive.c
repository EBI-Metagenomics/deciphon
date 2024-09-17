#include "aye.h"
#include "fs.h"
#include "imm_rnd.h"
#include "params.h"
#include "scan.h"
#include "test_utils.h"

#define HMMFILE "three.hmm"
#define DBFILE "test_massive.dcp"
#define PRODDIR "test_massive_product"

static void random_sequence_init(void);
static char const *random_sequence_next(void);

int main(void)
{
  aye_begin();
  setup_database(1, 0.01, HMMFILE, DBFILE);
  fs_rmtree(PRODDIR);
  random_sequence_init();
  char name[256] = {};
  struct params params = {};
  struct scan *scan = NULL;

  params.port = 51301;
  aye(params_setup(&params, 1, true, false) == 0);
  aye(scan = scan_new(params));
  aye(scan_open(scan, DBFILE) == 0);
  for (int i = 0; i < 10000; ++i)
  {
    snprintf(name, sizeof(name), "name%d", i);
    aye(scan_add(scan, i, name, random_sequence_next()) == 0);
  }
  aye(scan_run(scan, PRODDIR, NULL, NULL) == 0);
  aye(scan_progress(scan) == 100);
  printf("%ld\n", chksum(PRODDIR "/products.tsv"));
  aye(chksum(PRODDIR "/products.tsv") == 48347);
  aye(scan_close(scan) == 0);

  scan_del(scan);
  fs_rmtree(PRODDIR);
  cleanup_database(DBFILE);
  return aye_end();
}

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
