#include "protein_null.h"
#include "array_size.h"
#include "imm/lprob.h"
#include "imm/score_table.h"
#include "xtrans.h"

void protein_null_init(struct protein_null *x, struct imm_nuclt const *nuclt)
{
  nuclt_dist_init(&x->nuclt_dist, nuclt);
  x->RR = 0;
  for (size_t i = 0; i < array_size(x->emission); ++i)
    x->emission[i] = IMM_LPROB_NAN;
}

void protein_null_setup(struct protein_null *x, struct xtrans const *t)
{
  x->RR = t->RR;
}

void protein_null_absorb(struct protein_null *x, struct imm_score_table *st,
                         struct nuclt_dist const *nuclt_dist,
                         struct imm_state const *state)
{
  x->nuclt_dist = *nuclt_dist;
  imm_score_table_scores(st, state, x->emission);
}
