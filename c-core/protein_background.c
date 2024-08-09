#include "protein_background.h"
#include "array_size.h"
#include "imm_lprob.h"
#include "imm_score_table.h"

void protein_background_init(struct protein_background *x,
                             struct imm_nuclt const *nuclt)
{
  nuclt_dist_init(&x->nuclt_dist, nuclt);
  for (size_t i = 0; i < array_size(x->emission); ++i)
    x->emission[i] = IMM_LPROB_NAN;
}

void protein_background_absorb(struct protein_background *x,
                               struct model const *m,
                               struct imm_score_table *st)
{
  x->nuclt_dist = m->background.nuclt_dist;
  imm_score_table_scores(st, &m->background.state.super, x->emission);
}
