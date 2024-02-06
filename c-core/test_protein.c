#include "codec.h"
#include "imm/dna.h"
#include "imm/eseq.h"
#include "imm/gencode.h"
#include "imm/nuclt_code.h"
#include "imm/path.h"
#include "protein.h"
#include "setup_viterbi.h"
#include "state.h"
#include "trellis.h"
#include "vendor/minctest.h"
#include "viterbi.h"

static void test_protein_uniform(void);
static void test_protein_occupancy(void);

int main(void)
{
  test_protein_uniform();
  test_protein_occupancy();
  return lfails;
}

static int code_fn(int pos, int len, void *arg)
{
  struct imm_eseq const *seq = arg;
  return imm_eseq_get(seq, pos, len, 1);
}

static void test_protein_uniform(void)
{
  struct imm_path path = imm_path();
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

  char name[IMM_STATE_NAME_SIZE];

  eq(imm_eseq_setup(&eseq, &seq), 0);

  struct viterbi *task = viterbi_new();
  eq(viterbi_setup(task, protein.core_size), 0);
  eq(setup_viterbi(task, &protein), 0);
  close(viterbi_null(task, imm_eseq_size(&eseq), code_fn, (void *)&eseq),  48.9272687711);
  close(viterbi_cost(task, imm_eseq_size(&eseq), code_fn, (void *)&eseq), 55.59428153448);
  eq(viterbi_path(task, imm_eseq_size(&eseq), code_fn, (void *)&eseq), 0);
  imm_path_reset(&path);
  eq(trellis_unzip(viterbi_trellis(task), imm_eseq_size(&eseq), &path), 0);

  eq(imm_path_nsteps(&path), 14);

  eq(imm_path_step(&path, 0)->seqsize, 0);
  eq(imm_path_step(&path, 0)->state_id, STATE_S);
  state_name(imm_path_step(&path, 0)->state_id, name);
  cmp(name, "S");

  eq(imm_path_step(&path, 13)->seqsize, 0);
  eq(imm_path_step(&path, 13)->state_id, STATE_T);
  state_name(imm_path_step(&path, 13)->state_id, name);
  cmp(name, "T");

  struct codec codec = codec_init(&protein, &path);
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
  viterbi_del(task);
  protein_cleanup(&protein);
  imm_path_cleanup(&path);
}

static void test_protein_occupancy(void)
{
  struct imm_path path = imm_path();
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

  char name[IMM_STATE_NAME_SIZE];

  eq(imm_eseq_setup(&eseq, &seq), 0);

  struct viterbi *task = viterbi_new();
  eq(viterbi_setup(task, protein.core_size), 0);
  eq(setup_viterbi(task, &protein), 0);
  close(viterbi_null(task, imm_eseq_size(&eseq), code_fn, (void *)&eseq), 48.9272687711);
  close(viterbi_cost(task, imm_eseq_size(&eseq), code_fn, (void *)&eseq), 54.35543421312);
  eq(viterbi_path(task, imm_eseq_size(&eseq), code_fn, (void *)&eseq), 0);
  imm_path_reset(&path);
  eq(trellis_unzip(viterbi_trellis(task), imm_eseq_size(&eseq), &path), 0);

  eq(imm_path_nsteps(&path), 14);

  eq(imm_path_step(&path, 0)->seqsize, 0);
  eq(imm_path_step(&path, 0)->state_id, STATE_S);
  state_name(imm_path_step(&path, 0)->state_id, name);
  cmp(name, "S");

  eq(imm_path_step(&path, 13)->seqsize, 0);
  eq(imm_path_step(&path, 13)->state_id, STATE_T);
  state_name(imm_path_step(&path, 13)->state_id, name);
  cmp(name, "T");

  struct codec codec = codec_init(&protein, &path);
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
  viterbi_del(task);
  protein_cleanup(&protein);
  imm_path_cleanup(&path);
}
