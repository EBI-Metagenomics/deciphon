#ifndef DECIPHON_PROTEIN_H
#define DECIPHON_PROTEIN_H

#include "entry_dist.h"
#include "imm/imm.h"
#include "model.h"
#include "protein_background.h"
#include "protein_node.h"
#include "protein_null.h"

struct dcp_protein
{
  struct dcp_model_params params;

  char accession[32];
  imm_state_name *state_name;

  struct imm_frame_epsilon epsilon_frame;

  struct imm_score_table score_table;
  char consensus[DCP_MODEL_MAX + 1];

  float start_lprob;
  unsigned core_size;
  struct protein_null null;
  struct protein_background bg;
  struct dcp_protein_node *nodes;
  float *nodes_emission;
  struct dcp_xtrans xtrans;
  float *BMk;
};

void protein_init(struct dcp_protein *, struct dcp_model_params params);
int protein_set_accession(struct dcp_protein *, char const *accession);
void protein_setup(struct dcp_protein *, unsigned seq_size, bool multi_hits,
                   bool hmmer3_compat);
int protein_absorb(struct dcp_protein *, struct dcp_model *);
int protein_sample(struct dcp_protein *, unsigned seed, unsigned core_size);
void protein_cleanup(struct dcp_protein *);
void protein_dump(struct dcp_protein const *, FILE *restrict);

int protein_pack(struct dcp_protein const *, struct lip_file *);
int protein_unpack(struct dcp_protein *, struct lip_file *);

int protein_decode(struct dcp_protein const *, struct imm_seq const *,
                   unsigned state_id, struct imm_codon *);

#endif
