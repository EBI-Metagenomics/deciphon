#ifndef DECIPHON_P7_NODE_H
#define DECIPHON_P7_NODE_H

#include "imm/imm.h"
#include "nuclt_dist.h"
#include "p7_node_size.h"
#include "trans.h"

struct p7_node
{
  struct dcp_nuclt_dist nuclt_dist;
  struct dcp_trans trans;
  float emission[P7_NODE_SIZE];
};

struct imm_score_table;
struct imm_state;

void p7_node_absorb_emission(struct p7_node *, struct dcp_nuclt_dist const *,
                             struct imm_score_table *,
                             struct imm_state const *);
void p7_node_absorb_transition(struct p7_node *, struct dcp_trans const *);

#endif
