#ifndef DECIPHON_P7_H
#define DECIPHON_P7_H

#include "entry_dist.h"
#include "imm/imm.h"
#include "model.h"
#include "p7_background.h"
#include "p7_node.h"
#include "p7_null.h"

struct p7
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
  struct p7_node *nodes;
  struct dcp_xtrans xtrans;
  float *BMk;
};

void p7_init(struct p7 *, struct dcp_model_params params);
int p7_set_accession(struct p7 *, char const *accession);
void p7_setup(struct p7 *, unsigned seq_size, bool multi_hits,
              bool hmmer3_compat);
int p7_absorb(struct p7 *, struct dcp_model *);
int p7_sample(struct p7 *, unsigned seed, unsigned core_size);
void p7_dump(struct p7 const *, FILE *restrict);

int p7_pack(struct p7 const *, struct lip_file *);
int p7_unpack(struct p7 *, struct lip_file *);

#endif
