#ifndef DECIPHON_PROTEIN_ALTS_H
#define DECIPHON_PROTEIN_ALTS_H

#include "imm/imm.h"
#include "model.h"

struct dcp_protein_alt
{
  struct imm_dp dp;
  unsigned S;
  unsigned N;
  unsigned B;
  unsigned E;
  unsigned J;
  unsigned C;
  unsigned T;
};

struct dcp_protein_alts
{
  unsigned core_size;
  struct nuclt_dist *match_nuclt_dists;
  struct nuclt_dist insert_nuclt_dist;
  struct dcp_protein_alt zero;
  struct dcp_protein_alt full;
};

void dcp_protein_alts_init(struct dcp_protein_alts *,
                           struct imm_nuclt_code const *);
void dcp_protein_alts_setup(struct dcp_protein_alts *,
                            struct dcp_xtrans const *);
int dcp_protein_alts_absorb(struct dcp_protein_alts *, struct model *,
                            struct model_summary const *);
int dcp_protein_alts_pack(struct dcp_protein_alts const *, struct lip_file *);
int dcp_protein_alts_unpack(struct dcp_protein_alts *, struct lip_file *);
void dcp_protein_alts_cleanup(struct dcp_protein_alts *);

#endif
