#ifndef PROTEIN_NODE_H
#define PROTEIN_NODE_H

#include "imm/imm.h"
#include "nuclt_dist.h"
#include "trans.h"

struct protein_node
{
  struct nuclt_dist nuclt_dist;
  struct trans trans;
  float *emission;
};

void protein_node_absorb_emission(struct protein_node *, float *emission,
                                  struct nuclt_dist const *,
                                  struct imm_score_table *,
                                  struct imm_state const *);
void protein_node_absorb_transition(struct protein_node *,
                                    struct trans const *);

#endif
