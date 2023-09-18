#include "deciphon/codec.h"
#include "deciphon/p7.h"
#include "deciphon/vit.h"
#include "imm/imm.h"
#include "vendor/minctest.h"
#include "vit.h"

static void test_protein_uniform(void);
static void test_protein_occupancy(void);

int main(void)
{
  test_protein_uniform();
  test_protein_occupancy();
  return lfails;
}

static void test_protein_uniform(void)
{
  struct imm_amino const *amino = &imm_amino_iupac;
  struct imm_nuclt const *nuclt = &imm_dna_iupac.super;
  struct imm_nuclt_code code = {};
  imm_nuclt_code_init(&code, nuclt);

  struct imm_eseq eseq = {0};
  imm_eseq_init(&eseq, &code.super);

  struct dcp_model_params params = {
      .gencode = imm_gencode_get(1),
      .amino = amino,
      .code = &code,
      .entry_dist = DCP_ENTRY_DIST_UNIFORM,
      .epsilon = 0.1,
  };

  struct p7 p7 = {0};
  p7_init(&p7, params);
  p7_set_accession(&p7, "accession");
  eq(p7_sample(&p7, 1, 2), 0);

  char const str[] = "ATGAAACGCATTAGCACCACCATTACCACCAC";
  struct imm_seq seq = imm_seq(IMM_STR(str), &nuclt->super);

  p7_setup(&p7, imm_seq_size(&seq), true, false);

  eq(imm_eseq_setup(&eseq, &seq), 0);

  close(dcp_vit_null(&p7, &eseq), -48.9272687711);

  char name[IMM_STATE_NAME_SIZE];

  eq(imm_eseq_setup(&eseq, &seq), 0);

  struct dcp_viterbi_task task = {};
  dcp_viterbi_task_init(&task);
  close(dcp_vit(&p7, &eseq, &task), 0);
  close(task.score, -55.59428153448);

  eq(imm_path_nsteps(&task.path), 14U);

  eq(imm_path_step(&task.path, 0)->seqlen, 0);
  eq(imm_path_step(&task.path, 0)->state_id, STATE_S);
  dcp_state_name(imm_path_step(&task.path, 0)->state_id, name);
  cmp(name, "S");

  eq(imm_path_step(&task.path, 13)->seqlen, 0);
  eq(imm_path_step(&task.path, 13)->state_id, STATE_T);
  dcp_state_name(imm_path_step(&task.path, 13)->state_id, name);
  cmp(name, "T");

  struct dcp_codec codec = dcp_codec_init(&p7, &task.path);
  int rc = 0;

  struct imm_codon codons[10] = {
      IMM_CODON(nuclt, "ATG"), IMM_CODON(nuclt, "AAA"), IMM_CODON(nuclt, "CGC"),
      IMM_CODON(nuclt, "ATA"), IMM_CODON(nuclt, "GCA"), IMM_CODON(nuclt, "CCA"),
      IMM_CODON(nuclt, "CCT"), IMM_CODON(nuclt, "TAC"), IMM_CODON(nuclt, "CAC"),
      IMM_CODON(nuclt, "CAC"),
  };

  unsigned any = imm_abc_any_symbol_id(&nuclt->super);
  struct imm_codon codon = imm_codon(nuclt, any, any, any);
  int i = 0;
  while (!(rc = dcp_codec_next(&codec, &seq, &codon)))
  {
    if (dcp_codec_end(&codec)) break;
    eq(codons[i].a, codon.a);
    eq(codons[i].b, codon.b);
    eq(codons[i].c, codon.c);
    ++i;
  }
  eq(rc, 0);
  eq(i, 10);

  imm_eseq_cleanup(&eseq);
  dcp_viterbi_task_cleanup(&task);
  p7_cleanup(&p7);
}

static void test_protein_occupancy(void)
{
  struct imm_amino const *amino = &imm_amino_iupac;
  struct imm_nuclt const *nuclt = &imm_dna_iupac.super;
  struct imm_nuclt_code code;
  imm_nuclt_code_init(&code, nuclt);

  struct imm_eseq eseq = {0};
  imm_eseq_init(&eseq, &code.super);

  struct dcp_model_params params = {
      .gencode = imm_gencode_get(1),
      .amino = amino,
      .code = &code,
      .entry_dist = DCP_ENTRY_DIST_OCCUPANCY,
      .epsilon = 0.1,
  };

  struct p7 p7 = {0};
  p7_init(&p7, params);
  p7_set_accession(&p7, "accession");
  eq(p7_sample(&p7, 1, 2), 0);

  char const str[] = "ATGAAACGCATTAGCACCACCATTACCACCAC";
  struct imm_seq seq = imm_seq(imm_str(str), &nuclt->super);

  p7_setup(&p7, imm_seq_size(&seq), true, false);

  eq(imm_eseq_setup(&eseq, &seq), 0);

  close(dcp_vit_null(&p7, &eseq), -48.9272687711);

  char name[IMM_STATE_NAME_SIZE];

  eq(imm_eseq_setup(&eseq, &seq), 0);

  struct dcp_viterbi_task task = {};
  dcp_viterbi_task_init(&task);
  close(dcp_vit(&p7, &eseq, &task), 0);
  close(task.score, -54.35543421312);

  eq(imm_path_nsteps(&task.path), 14U);

  eq(imm_path_step(&task.path, 0)->seqlen, 0);
  eq(imm_path_step(&task.path, 0)->state_id, STATE_S);
  dcp_state_name(imm_path_step(&task.path, 0)->state_id, name);
  cmp(name, "S");

  eq(imm_path_step(&task.path, 13)->seqlen, 0);
  eq(imm_path_step(&task.path, 13)->state_id, STATE_T);
  dcp_state_name(imm_path_step(&task.path, 13)->state_id, name);
  cmp(name, "T");

  struct dcp_codec codec = dcp_codec_init(&p7, &task.path);
  int rc = 0;

  struct imm_codon codons[10] = {
      IMM_CODON(nuclt, "ATG"), IMM_CODON(nuclt, "AAA"), IMM_CODON(nuclt, "CGC"),
      IMM_CODON(nuclt, "ATA"), IMM_CODON(nuclt, "GCA"), IMM_CODON(nuclt, "CCA"),
      IMM_CODON(nuclt, "CCT"), IMM_CODON(nuclt, "TAC"), IMM_CODON(nuclt, "CAC"),
      IMM_CODON(nuclt, "CAC"),
  };

  unsigned any = imm_abc_any_symbol_id(&nuclt->super);
  struct imm_codon codon = imm_codon(nuclt, any, any, any);
  int i = 0;
  while (!(rc = dcp_codec_next(&codec, &seq, &codon)))
  {
    if (dcp_codec_end(&codec)) break;
    eq(codons[i].a, codon.a);
    eq(codons[i].b, codon.b);
    eq(codons[i].c, codon.c);
    ++i;
  }
  eq(rc, 0);
  eq(i, 10);

  imm_eseq_cleanup(&eseq);
  dcp_viterbi_task_cleanup(&task);
  p7_cleanup(&p7);
}
