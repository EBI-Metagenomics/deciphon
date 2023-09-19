#ifndef DECIPHON_PROTEIN_BACKGROUND_H
#define DECIPHON_PROTEIN_BACKGROUND_H

#include "model.h"
#include "nuclt_dist.h"
#include "protein_node_size.h"

struct dcp_protein_background
{
  struct dcp_nuclt_dist nuclt_dist;
  float emission[PROTEIN_NODE_SIZE];
};

struct imm_score_table;
struct dcp_model;

void dcp_protein_background_init(struct dcp_protein_background *);
void dcp_protein_background_absorb(struct dcp_protein_background *,
                                   struct dcp_model const *,
                                   struct imm_score_table *);

#endif
