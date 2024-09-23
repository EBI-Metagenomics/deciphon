#ifndef PROTEIN_H
#define PROTEIN_H

#include "imm_score_table.h"
#include "model.h"
#include "protein_background.h"
#include "protein_null.h"

struct protein
{
  struct model_params params;

  char accession[32];
  struct imm_score_table score_table;
  char consensus[MODEL_MAX + 1];

  int core_size;
  bool has_ga;
  bool multi_hits;
  bool hmmer3_compat;
  struct protein_null null;
  struct protein_background bg;
  struct protein_node *nodes;
  float *emission;
  struct xtrans xtrans;
  float *BMk;
};

struct lio_writer;
struct lio_reader;
struct viterbi;

void protein_init(struct protein *);
void protein_setup(struct protein *, struct model_params params, bool multi_hits, bool hmmer3_compat);
int  protein_set_accession(struct protein *, char const *accession);
void protein_reset(struct protein *, int seq_size);
int  protein_absorb(struct protein *, struct model *);
int  protein_sample(struct protein *, int seed, int core_size);
void protein_cleanup(struct protein *);
void protein_dump(struct protein const *, FILE *);
int  protein_pack(struct protein const *, struct lio_writer *);
int  protein_unpack(struct protein *, struct lio_reader *);
int  protein_decode(struct protein const *, struct imm_seq const *, int state_id, struct imm_codon *);
int  protein_setup_viterbi(struct protein const *, struct viterbi *);

#endif
