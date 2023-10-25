#include "fs.h"
#include "imm/imm.h"
#include "press.h"
#include "scan.h"
#include "scan_params.h"
#include "test_consensus.h"
#include "test_utils.h"
#include "vendor/minctest.h"

#define SIZE 500000
#define HMMFILE "minifam.hmm"
#define DBFILE "test_window.dcp"
#define PRODDIR "test_window"

int main(void)
{
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

  struct scan_params params = {};
  struct scan *scan = NULL;

  eq(scan_params_setup(&params, 1, true, false), 0);
  ok(scan = scan_new(params));
  eq(scan_open(scan, DBFILE), 0);
  eq(scan_add(scan, sequences[0].id, sequences[0].name, seq), 0);
  eq(scan_run(scan, PRODDIR), 0);
  eq(chksum(PRODDIR "/products.tsv"), 41833);
  eq(scan_close(scan), 0);

  scan_del(scan);
  fs_rmtree(PRODDIR);
  cleanup_database(DBFILE);
  return lfails;
}