#if 0
#include "c11threads.h"
#include "fs.h"
#include "imm/imm.h"
#include "params.h"
#include "press.h"
#include "scan.h"
#include "test_consensus.h"
#include "test_utils.h"
#include "vendor/minctest.h"
#include <stdatomic.h>

#define SIZE 50000
#define HMMFILE "Pfam-A.500.hmm"
#define DBFILE "test_threads.dcp"
#define PRODDIR "test_threads_prod"

static thrd_t progress_thread = {0};
static atomic_bool progress_continue = true;

int progress_entry(void *arg)
{
  struct scan const *scan = arg;
  while (progress_continue)
  {
    fprintf(stderr, "%d%% ", scan_progress(scan));
    thrd_sleep(&(struct timespec){.tv_nsec = 10000000}, NULL);
  }
  fprintf(stderr, "%d%%\n", scan_progress(scan));
  return 0;
}

void progress_start(struct scan const *scan)
{
  progress_continue = true;
  eq(thrd_create(&progress_thread, &progress_entry, (void *)scan),
     thrd_success);
}

void progress_stop(void)
{
  progress_continue = false;
  eq(thrd_join(progress_thread, NULL), thrd_success);
}

int main(void)
{
  setup_database(1, 0.01, HMMFILE, DBFILE);
  fs_rmtree(PRODDIR);

  struct imm_rnd rnd = imm_rnd(591);
  static char seq[SIZE + 1];
  for (int i = 0; i < SIZE; ++i)
  {
    seq[i] = sequences[0].data[i % strlen(sequences[0].data)];
    seq[i] = (i % 10 == 0) ? "ACGT"[imm_rnd_u64(&rnd) % 4] : seq[i];
    seq[i] = (i % 10 == 5) ? "ACGT"[imm_rnd_u64(&rnd) % 4] : seq[i];
    seq[i] = (i % 10 == 3) ? "ACGT"[imm_rnd_u64(&rnd) % 4] : seq[i];
    seq[i] = (i % 10 == 9) ? "ACGT"[imm_rnd_u64(&rnd) % 4] : seq[i];
    seq[i] = (i % 10 == 4) ? "ACGT"[imm_rnd_u64(&rnd) % 4] : seq[i];
    seq[i] = (i % 10 == 1) ? "ACGT"[imm_rnd_u64(&rnd) % 4] : seq[i];
    seq[i] = (i % 10 == 2) ? "ACGT"[imm_rnd_u64(&rnd) % 4] : seq[i];
  }
  seq[SIZE] = '\0';

  struct params params = {};
  struct scan *scan = NULL;

  eq(params_setup(&params, 4, true, false), 0);
  ok(scan = scan_new(params));

  eq(scan_open(scan, DBFILE), 0);
  progress_start(scan);

  eq(scan_add(scan, sequences[0].id, sequences[0].name, seq), 0);
  eq(scan_run(scan, PRODDIR), 0);

  progress_stop();
  eq(scan_close(scan), 0);

  eq(scan_progress(scan), 100);
  eq(chksum(PRODDIR "/products.tsv"), 44039);

  scan_del(scan);
  fs_rmtree(PRODDIR);
  // cleanup_database(DBFILE);
  return lfails;
}
#else
int main(void) { return 0; }
#endif
