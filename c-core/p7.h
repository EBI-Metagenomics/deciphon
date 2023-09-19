#ifndef DECIPHON_P7_H
#define DECIPHON_P7_H

#include "entry_dist.h"
#include "imm/imm.h"
#include "model.h"
#include "p7_background.h"
#include "p7_node.h"
#include "p7_null.h"

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
  struct p7_null null;
  struct p7_background bg;
  struct dcp_protein_node *nodes;
  float *nodes_emission;
  struct dcp_xtrans xtrans;
  float *BMk;
};

void p7_init(struct dcp_protein *, struct dcp_model_params params);
int p7_set_accession(struct dcp_protein *, char const *accession);
void p7_setup(struct dcp_protein *, unsigned seq_size, bool multi_hits,
              bool hmmer3_compat);
int p7_absorb(struct dcp_protein *, struct dcp_model *);
int p7_sample(struct dcp_protein *, unsigned seed, unsigned core_size);
void p7_cleanup(struct dcp_protein *);
void p7_dump(struct dcp_protein const *, FILE *restrict);

int p7_pack(struct dcp_protein const *, struct lip_file *);
int p7_unpack(struct dcp_protein *, struct lip_file *);

int p7_decode(struct dcp_protein const *, struct imm_seq const *, unsigned state_id,
              struct imm_codon *);

#endif
