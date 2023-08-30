#ifndef DECIPHON_P7_BACKGROUND_H
#define DECIPHON_P7_BACKGROUND_H

#include "model.h"
#include "nuclt_dist.h"
#include "p7_node_size.h"

struct p7_background
{
  struct dcp_nuclt_dist nuclt_dist;
  float emission[P7_NODE_SIZE];
};

struct imm_score_table;
struct dcp_model;

void p7_background_init(struct p7_background *);
void p7_background_absorb(struct p7_background *, struct dcp_model const *,
                          struct imm_score_table *);

#endif
