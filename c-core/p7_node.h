#ifndef DECIPHON_P7_NODE_H
#define DECIPHON_P7_NODE_H

#include "imm/imm.h"
#include "nuclt_dist.h"
#include "trans.h"

struct dcp_protein_node
{
  struct dcp_nuclt_dist nuclt_dist;
  struct dcp_trans trans;
  float *emission;
};

void p7_node_absorb_emission(struct dcp_protein_node *, float *emission,
                             struct dcp_nuclt_dist const *,
                             struct imm_score_table *,
                             struct imm_state const *);
void p7_node_absorb_transition(struct dcp_protein_node *,
                               struct dcp_trans const *);

#endif
