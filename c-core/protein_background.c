#include "protein_background.h"
#include "array_size.h"
#include "imm/score_table.h"

void dcp_protein_background_init(struct dcp_protein_background *x)
{
  dcp_nuclt_dist_init(&x->nuclt_dist, &imm_dna_iupac.super);
  for (size_t i = 0; i < array_size(x->emission); ++i)
    x->emission[i] = IMM_LPROB_NAN;
}

void dcp_protein_background_absorb(struct dcp_protein_background *x,
                                   struct dcp_model const *m,
                                   struct imm_score_table *st)
{
  x->nuclt_dist = m->background.nuclt_dist;
  imm_score_table_scores(st, &m->background.state.super, x->emission);
}
