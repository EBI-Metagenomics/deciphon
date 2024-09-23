#include "aye.h"
#include "batch.h"
#include "fs.h"
#include "imm_rnd.h"
#include "scan.h"
#include "test_consensus.h"
#include "test_utils.h"
#include <string.h>

#define SIZE 150000
#define HMMFILE "minifam.hmm"
#define DBFILE "test_window.dcp"
#define PRODDIR "test_window_prod"
#define PORT 51300
#define NUM_THREADS 1
#define MULTI_HITS true
#define HMMER3_COMPAT false

int main(void)
{
  aye_begin();
  setup_database(1, 0.1, HMMFILE, DBFILE);
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

  struct scan *scan = NULL;
  struct batch *batch = NULL;
  aye(scan = scan_new());
  aye(batch = batch_new());
  aye(scan_setup(scan, DBFILE, PORT, NUM_THREADS, MULTI_HITS, HMMER3_COMPAT) ==
      0);
  aye(batch_add(batch, sequences[0].id, sequences[0].name, seq) == 0);
  aye(scan_run(scan, batch, PRODDIR, NULL, NULL) == 0);
  aye(scan_progress(scan) == 100);
  aye(chksum(PRODDIR "/products.tsv") == 9910);

  batch_del(batch);
  scan_del(scan);
  fs_rmtree(PRODDIR);
  cleanup_database(DBFILE);
  return aye_end();
}
