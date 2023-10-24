#include "codec.h"
#include "imm/imm.h"
#include "protein.h"
#include "vendor/minctest.h"
#include "viterbi.h"
#include "viterbi_task.h"

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

  struct model_params params = {
      .gencode = imm_gencode_get(1),
      .amino = amino,
      .code = &code,
      .entry_dist = ENTRY_DIST_UNIFORM,
      .epsilon = 0.1,
  };

  struct protein protein = {0};
  protein_init(&protein);
  protein_setup(&protein, params);
  protein_set_accession(&protein, "accession");
  eq(protein_sample(&protein, 1, 2), 0);

  char const str[] = "ATGAAACGCATTAGCACCACCATTACCACCAC";
  struct imm_seq seq = imm_seq_unsafe(imm_str(str), &nuclt->super);

  protein_reset(&protein, imm_seq_size(&seq), true, false);

  eq(imm_eseq_setup(&eseq, &seq), 0);

  close(viterbi_null(&protein, &eseq), -48.9272687711);

  char name[IMM_STATE_NAME_SIZE];

  eq(imm_eseq_setup(&eseq, &seq), 0);

  struct viterbi_task task = {};
  viterbi_task_init(&task);
  viterbi_task_setup_protein(&task, &protein);
  viterbi_task_setup_path(&task, imm_eseq_size(&eseq));
  close(viterbi_alt(&task, &eseq, false), -55.59428153448);
  eq(viterbi_alt_extract_path(&task, imm_eseq_size(&eseq)), 0);

  eq(imm_path_nsteps(&task.path), 14U);

  eq(imm_path_step(&task.path, 0)->seqsize, 0);
  eq(imm_path_step(&task.path, 0)->state_id, STATE_S);
  state_name(imm_path_step(&task.path, 0)->state_id, name);
  cmp(name, "S");

  eq(imm_path_step(&task.path, 13)->seqsize, 0);
  eq(imm_path_step(&task.path, 13)->state_id, STATE_T);
  state_name(imm_path_step(&task.path, 13)->state_id, name);
  cmp(name, "T");

  struct codec codec = codec_init(&protein, &task.path);
  int rc = 0;

  struct imm_codon codons[10] = {
      IMM_CODON(nuclt, "ATG"), IMM_CODON(nuclt, "AAA"), IMM_CODON(nuclt, "CGC"),
      IMM_CODON(nuclt, "ATA"), IMM_CODON(nuclt, "GCA"), IMM_CODON(nuclt, "CCA"),
      IMM_CODON(nuclt, "CCT"), IMM_CODON(nuclt, "TAC"), IMM_CODON(nuclt, "CAC"),
      IMM_CODON(nuclt, "CAC"),
  };

  int any = imm_abc_any_symbol_id(&nuclt->super);
  struct imm_codon codon = imm_codon(nuclt, any, any, any);
  int i = 0;
  while (!(rc = codec_next(&codec, &seq, &codon)))
  {
    if (codec_end(&codec)) break;
    eq(codons[i].a, codon.a);
    eq(codons[i].b, codon.b);
    eq(codons[i].c, codon.c);
    ++i;
  }
  eq(rc, 0);
  eq(i, 10);

  imm_eseq_cleanup(&eseq);
  viterbi_task_cleanup(&task);
  protein_cleanup(&protein);
}

static void test_protein_occupancy(void)
{
  struct imm_amino const *amino = &imm_amino_iupac;
  struct imm_nuclt const *nuclt = &imm_dna_iupac.super;
  struct imm_nuclt_code code;
  imm_nuclt_code_init(&code, nuclt);

  struct imm_eseq eseq = {0};
  imm_eseq_init(&eseq, &code.super);

  struct model_params params = {
      .gencode = imm_gencode_get(1),
      .amino = amino,
      .code = &code,
      .entry_dist = ENTRY_DIST_OCCUPANCY,
      .epsilon = 0.1,
  };

  struct protein protein = {0};
  protein_init(&protein);
  protein_setup(&protein, params);
  protein_set_accession(&protein, "accession");
  eq(protein_sample(&protein, 1, 2), 0);

  char const str[] = "ATGAAACGCATTAGCACCACCATTACCACCAC";
  struct imm_seq seq = imm_seq_unsafe(imm_str(str), &nuclt->super);

  protein_reset(&protein, imm_seq_size(&seq), true, false);

  eq(imm_eseq_setup(&eseq, &seq), 0);

  close(viterbi_null(&protein, &eseq), -48.9272687711);

  char name[IMM_STATE_NAME_SIZE];

  eq(imm_eseq_setup(&eseq, &seq), 0);

  struct viterbi_task task = {};
  viterbi_task_init(&task);
  viterbi_task_setup_protein(&task, &protein);
  viterbi_task_setup_path(&task, imm_eseq_size(&eseq));
  close(viterbi_alt(&task, &eseq, false), -54.35543421312);
  eq(viterbi_alt_extract_path(&task, imm_eseq_size(&eseq)), 0);

  eq(imm_path_nsteps(&task.path), 14U);

  eq(imm_path_step(&task.path, 0)->seqsize, 0);
  eq(imm_path_step(&task.path, 0)->state_id, STATE_S);
  state_name(imm_path_step(&task.path, 0)->state_id, name);
  cmp(name, "S");

  eq(imm_path_step(&task.path, 13)->seqsize, 0);
  eq(imm_path_step(&task.path, 13)->state_id, STATE_T);
  state_name(imm_path_step(&task.path, 13)->state_id, name);
  cmp(name, "T");

  struct codec codec = codec_init(&protein, &task.path);
  int rc = 0;

  struct imm_codon codons[10] = {
      IMM_CODON(nuclt, "ATG"), IMM_CODON(nuclt, "AAA"), IMM_CODON(nuclt, "CGC"),
      IMM_CODON(nuclt, "ATA"), IMM_CODON(nuclt, "GCA"), IMM_CODON(nuclt, "CCA"),
      IMM_CODON(nuclt, "CCT"), IMM_CODON(nuclt, "TAC"), IMM_CODON(nuclt, "CAC"),
      IMM_CODON(nuclt, "CAC"),
  };

  int any = imm_abc_any_symbol_id(&nuclt->super);
  struct imm_codon codon = imm_codon(nuclt, any, any, any);
  int i = 0;
  while (!(rc = codec_next(&codec, &seq, &codon)))
  {
    if (codec_end(&codec)) break;
    eq(codons[i].a, codon.a);
    eq(codons[i].b, codon.b);
    eq(codons[i].c, codon.c);
    ++i;
  }
  eq(rc, 0);
  eq(i, 10);

  imm_eseq_cleanup(&eseq);
  viterbi_task_cleanup(&task);
  protein_cleanup(&protein);
}
