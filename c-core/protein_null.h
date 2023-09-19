#ifndef DECIPHON_PROTEIN_NULL_H
#define DECIPHON_PROTEIN_NULL_H

#include "nuclt_dist.h"
#include "protein_node_size.h"

struct protein_null
{
  struct dcp_nuclt_dist nuclt_dist;
  float RR;
  float emission[PROTEIN_NODE_SIZE];
};

struct imm_score_table;
struct dcp_xtrans;
struct dcp_model;

void protein_null_init(struct protein_null *);
void protein_null_setup(struct protein_null *, struct dcp_xtrans const *);
void protein_null_absorb(struct protein_null *, struct imm_score_table *,
                         struct dcp_nuclt_dist const *,
                         struct imm_state const *);

#endif
