#ifndef DECIPHON_P7_NULL_H
#define DECIPHON_P7_NULL_H

#include "nuclt_dist.h"
#include "p7_node_size.h"

struct p7_null
{
  struct dcp_nuclt_dist nuclt_dist;
  float RR;
  float emission[P7_NODE_SIZE];
};

struct imm_score_table;
struct dcp_xtrans;
struct dcp_model;

void p7_null_init(struct p7_null *);
void p7_null_setup(struct p7_null *, struct dcp_xtrans const *);
void p7_null_absorb(struct p7_null *, struct imm_score_table *,
                    struct dcp_nuclt_dist const *, struct imm_state const *);

#endif
