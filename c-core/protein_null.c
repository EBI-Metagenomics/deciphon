#include "protein_null.h"
#include "expect.h"
#include "imm/score_table.h"
#include "xtrans.h"
#include <stdlib.h>

void protein_null_init(struct protein_null *x) { x->RR = 0; }

void protein_null_setup(struct protein_null *x, struct dcp_xtrans const *t)
{
  x->RR = t->RR;
}

void protein_null_absorb(struct protein_null *x, struct imm_score_table *st,
                    struct dcp_nuclt_dist const *nuclt_dist,
                    struct imm_state const *state)
{
  x->nuclt_dist = *nuclt_dist;
  imm_score_table_scores(st, state, x->emission);
}
