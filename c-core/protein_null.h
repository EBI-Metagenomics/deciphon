#ifndef PROTEIN_NULL_H
#define PROTEIN_NULL_H

#include "nuclt_dist.h"
#include "protein_node_size.h"

struct protein_null
{
  struct nuclt_dist nuclt_dist;
  float RR;
  float emission[PROTEIN_NODE_SIZE];
};

struct imm_score_table;
struct xtrans;
struct model;

// clang-format off
void protein_null_init(struct protein_null *, struct imm_nuclt const *);
void protein_null_setup(struct protein_null *, struct xtrans const *);
void protein_null_absorb(struct protein_null *, struct imm_score_table *, struct nuclt_dist const *, struct imm_state const *);
// clang-format on

#endif
