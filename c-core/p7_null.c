#include "p7_null.h"
#include "expect.h"
#include "imm/score_table.h"
#include "xtrans.h"
#include <stdlib.h>

void p7_null_init(struct p7_null *x) { x->RR = 0; }

void p7_null_setup(struct p7_null *x, struct dcp_xtrans const *t)
{
  x->RR = t->RR;
}

void p7_null_absorb(struct p7_null *x, struct imm_score_table *st,
                    struct dcp_nuclt_dist const *nuclt_dist,
                    struct imm_state const *state)
{
  x->nuclt_dist = *nuclt_dist;
  imm_score_table_scores(st, state, x->emission);
}
