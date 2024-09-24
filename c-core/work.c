#include "work.h"
#include "array_size_field.h"
#include "decoder.h"
#include "defer_return.h"
#include "error.h"
#include "protein.h"
#include "viterbi.h"
#include "xstrcpy.h"
#include <stddef.h>
#include <string.h>

void work_init(struct work *x)
{
  xtrans_init(&x->xtrans);
  x->multi_hits    = true;
  x->hmmer3_compat = false;
  x->core_size     = -1;
  memset(x->accession, 0, array_size_field(struct work, accession));
  decoder_init(&x->decoder);
  x->viterbi       = NULL;
}

int work_setup(struct work *x, struct protein *protein)
{
  int rc = 0;
  x->multi_hits    = protein->multi_hits;
  x->hmmer3_compat = protein->hmmer3_compat;
  x->xtrans        = protein->xtrans;
  x->core_size     = protein->core_size;

  int size = array_size_field(struct work, accession);
  if (xstrcpy(x->accession, protein->accession, size))   defer_return(DCP_ELONGACCESSION);
  if (!x->viterbi && !(x->viterbi = viterbi_new()))      defer_return(DCP_ENOMEM);
  if ((rc = decoder_setup(&x->decoder, protein)))        defer_return(rc);
  if ((rc = protein_setup_viterbi(protein, x->viterbi))) defer_return(rc);

  return rc;

defer:
  viterbi_del(x->viterbi);
  x->viterbi = NULL;
  decoder_cleanup(&x->decoder);
  return rc;
}

void work_reset(struct work *x, int seq_size)
{
  xtrans_setup(&x->xtrans, x->multi_hits, x->hmmer3_compat, seq_size);
  xtrans_setup_viterbi(&x->xtrans, x->viterbi);
}

void work_cleanup(struct work *x)
{
  decoder_cleanup(&x->decoder);
  viterbi_del(x->viterbi);
  x->viterbi = NULL;
}
