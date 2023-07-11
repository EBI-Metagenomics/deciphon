#ifndef DECIPHON_PROTEIN_NULL_H
#define DECIPHON_PROTEIN_NULL_H

#include "imm/imm.h"
#include "model.h"

struct dcp_protein_null
{
  struct dcp_nuclt_dist nuclt_dist;
  struct imm_dp dp;
  unsigned R;
};

void dcp_protein_null_init(struct dcp_protein_null *,
                           struct imm_nuclt_code const *);
void dcp_protein_null_setup(struct dcp_protein_null *,
                            struct dcp_xtrans const *);
int dcp_protein_null_absorb(struct dcp_protein_null *, struct dcp_model const *,
                            struct dcp_model_summary const *);
int dcp_protein_null_pack(struct dcp_protein_null const *, struct lip_file *);
int dcp_protein_null_unpack(struct dcp_protein_null *, struct lip_file *);
void dcp_protein_null_cleanup(struct dcp_protein_null *);

#endif
