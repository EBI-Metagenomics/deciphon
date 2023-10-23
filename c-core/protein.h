#ifndef PROTEIN_H
#define PROTEIN_H

#include "deciphon_limits.h"
#include "imm/imm.h"
#include "model.h"
#include "protein_background.h"
#include "protein_node.h"
#include "protein_null.h"

struct protein
{
  struct model_params params;

  char accession[DCP_ACCESSION_SIZE];
  struct imm_score_table score_table;
  char consensus[DCP_MODEL_MAX + 1];

  int core_size;
  struct protein_null null;
  struct protein_background bg;
  struct protein_node *nodes;
  float *emission;
  struct xtrans xtrans;
  float *BMk;
};

// clang-format off
void protein_init(struct protein *, struct model_params params);
int  protein_set_accession(struct protein *, char const *accession);
void protein_setup(struct protein *, int seq_size, bool multi_hits,
                   bool hmmer3_compat);
int  protein_absorb(struct protein *, struct model *);
int  protein_sample(struct protein *, int seed, int core_size);
void protein_cleanup(struct protein *);
void protein_dump(struct protein const *, FILE *);
int  protein_pack(struct protein const *, struct lip_file *);
int  protein_unpack(struct protein *, struct lip_file *);
int  protein_decode(struct protein const *, struct imm_seq const *, int state_id,
                    struct imm_codon *);
// clang-format on

#endif
