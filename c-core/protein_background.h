#ifndef PROTEIN_BACKGROUND_H
#define PROTEIN_BACKGROUND_H

#include "model.h"
#include "nuclt_dist.h"
#include "protein_node_size.h"

struct protein_background
{
  struct nuclt_dist nuclt_dist;
  float emission[PROTEIN_NODE_SIZE];
};

struct imm_score_table;
struct model;

void protein_background_init(struct protein_background *);
void protein_background_absorb(struct protein_background *,
                               struct model const *, struct imm_score_table *);

#endif
