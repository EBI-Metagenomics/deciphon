#include "model.h"
#include "hope.h"
#include "imm/imm.h"
#include "protein.h"

int main(void)
{
  unsigned core_size = 3;
  struct imm_amino const *amino = &imm_amino_iupac;
  struct imm_nuclt const *nuclt = &imm_dna_iupac.super;
  struct imm_nuclt_code code;
  imm_nuclt_code_init(&code, nuclt);
  float null_lprobs[IMM_AMINO_SIZE];
  float null_lodds[IMM_AMINO_SIZE];
  float match_lprobs1[IMM_AMINO_SIZE];
  float match_lprobs2[IMM_AMINO_SIZE];
  float match_lprobs3[IMM_AMINO_SIZE];
  struct dcp_trans t[4];

  struct imm_rnd rnd = imm_rnd(942);
  imm_lprob_sample(&rnd, IMM_AMINO_SIZE, null_lprobs);
  imm_lprob_sample(&rnd, IMM_AMINO_SIZE, null_lodds);
  imm_lprob_sample(&rnd, IMM_AMINO_SIZE, match_lprobs1);
  imm_lprob_sample(&rnd, IMM_AMINO_SIZE, match_lprobs2);
  imm_lprob_sample(&rnd, IMM_AMINO_SIZE, match_lprobs3);

  for (unsigned i = 0; i < 4; ++i)
  {
    imm_lprob_sample(&rnd, TRANS_SIZE, t[i].data);
    imm_lprob_normalize(TRANS_SIZE, t[i].data);
  }

  struct dcp_model model;
  dcp_model_init(&model, imm_gencode_get(1), amino, &code, ENTRY_DIST_OCCUPANCY,
                 0.01, null_lprobs);

  eq(dcp_model_setup(&model, core_size), 0);

  eq(dcp_model_add_node(&model, match_lprobs1, '-'), 0);
  eq(dcp_model_add_node(&model, match_lprobs2, '-'), 0);
  eq(dcp_model_add_node(&model, match_lprobs3, '-'), 0);

  eq(dcp_model_add_trans(&model, t[0]), 0);
  eq(dcp_model_add_trans(&model, t[1]), 0);
  eq(dcp_model_add_trans(&model, t[2]), 0);
  eq(dcp_model_add_trans(&model, t[3]), 0);

  struct dcp_protein protein = {0};
  dcp_protein_init(&protein, NULL, amino, &code, ENTRY_DIST_OCCUPANCY, 0.01);
  dcp_protein_set_accession(&protein, "accession");

  eq(dcp_protein_absorb(&protein, &model), 0);

  dcp_protein_cleanup(&protein);
  dcp_model_del(&model);
  return hope_status();
}
