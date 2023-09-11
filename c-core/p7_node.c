#include "p7_node.h"
#include "imm/score_table.h"

void p7_node_absorb_emission(struct p7_node *x, float *emission,
                             struct dcp_nuclt_dist const *nucltd,
                             struct imm_score_table *st,
                             struct imm_state const *state)
{
  x->nuclt_dist = *nucltd;
  imm_score_table_scores(st, state, emission);
  x->emission = emission;
}

void p7_node_absorb_transition(struct p7_node *x, struct dcp_trans const *trans)
{
  x->trans = *trans;
}
