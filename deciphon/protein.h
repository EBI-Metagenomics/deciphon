#ifndef DECIPHON_PROTEIN_H
#define DECIPHON_PROTEIN_H

#include "imm/imm.h"
#include "model.h"
#include "protein_alts.h"
#include "protein_null.h"

struct lip_file;
struct dcp_db_reader;

struct dcp_protein
{
  struct imm_gencode const *gencode;
  char accession[32];
  imm_state_name *state_name;
  struct imm_code const *imm_code;

  struct imm_amino const *amino;
  struct imm_nuclt_code const *nuclt_code;
  enum entry_dist entry_dist;
  float epsilon;
  struct imm_frame_epsilon epsilon_frame;
  char consensus[MODEL_MAX + 1];

  struct dcp_protein_null null;
  struct dcp_protein_alts alts;
};

void dcp_protein_init(struct dcp_protein *, struct imm_gencode const *,
                      struct imm_amino const *, struct imm_nuclt_code const *,
                      enum entry_dist, float epsilon);

int dcp_protein_set_accession(struct dcp_protein *, char const *);

void dcp_protein_setup(struct dcp_protein *, unsigned seq_size, bool multi_hits,
                       bool hmmer3_compat);

int dcp_protein_absorb(struct dcp_protein *, struct dcp_model *model);

int dcp_protein_sample(struct dcp_protein *, unsigned seed, unsigned core_size);

int dcp_protein_decode(struct dcp_protein const *, struct imm_seq const *,
                       unsigned state_id, struct imm_codon *codon);

void dcp_protein_write_dot(struct dcp_protein const *, struct imm_dp const *,
                           FILE *);

int dcp_protein_pack(struct dcp_protein const *, struct lip_file *file);
int dcp_protein_unpack(struct dcp_protein *, struct lip_file *file);

void dcp_protein_cleanup(struct dcp_protein *);

#endif
