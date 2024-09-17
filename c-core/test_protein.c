#include "aye.h"
#include "codec.h"
#include "imm_dna.h"
#include "imm_eseq.h"
#include "imm_gencode.h"
#include "imm_nuclt_code.h"
#include "imm_path.h"
#include "near.h"
#include "protein.h"
#include "state.h"
#include "trellis.h"
#include "viterbi.h"
#include <string.h>

static void test_protein_uniform(void);
static void test_protein_occupancy(void);

int main(void)
{
  aye_begin();
  test_protein_uniform();
  test_protein_occupancy();
  return aye_end();
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
  aye(protein_sample(&protein, 1, 2) == 0);

  char const str[] = "ATGAAACGCATTAGCACCACCATTACCACCAC";
  struct imm_seq seq = imm_seq_unsafe(imm_str(str), &nuclt->super);

  protein_reset(&protein, imm_seq_size(&seq), true, false);

  aye(imm_eseq_setup(&eseq, &seq) == 0);

  char name[IMM_STATE_NAME_SIZE];

  aye(imm_eseq_setup(&eseq, &seq) == 0);

  struct viterbi *viterbi = viterbi_new();
  aye(viterbi_setup(viterbi, protein.core_size) == 0);
  aye(protein_setup_viterbi(&protein, viterbi) == 0);
  aye(near(viterbi_null(viterbi, imm_eseq_size(&eseq), code_fn, (void *)&eseq),  48.9272687711));
  aye(near(viterbi_cost(viterbi, imm_eseq_size(&eseq), code_fn, (void *)&eseq), 55.59428153448));
  aye(viterbi_path(viterbi, imm_eseq_size(&eseq), code_fn, (void *)&eseq) == 0);
  imm_path_reset(&path);
  aye(trellis_unzip(viterbi_trellis(viterbi), imm_eseq_size(&eseq), &path) == 0);

  aye(imm_path_nsteps(&path) == 14);

  aye(imm_path_step(&path, 0)->seqsize == 0);
  aye(imm_path_step(&path, 0)->state_id == STATE_S);
  state_name(imm_path_step(&path, 0)->state_id, name);
  aye(strcmp(name, "S") == 0);

  aye(imm_path_step(&path, 13)->seqsize == 0);
  aye(imm_path_step(&path, 13)->state_id == STATE_T);
  state_name(imm_path_step(&path, 13)->state_id, name);
  aye(strcmp(name, "T") == 0);

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
    aye(codons[i].a == codon.a);
    aye(codons[i].b == codon.b);
    aye(codons[i].c == codon.c);
    ++i;
  }
  aye(rc == 0);
  aye(i == 10);

  imm_eseq_cleanup(&eseq);
  viterbi_del(viterbi);
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
  aye(protein_sample(&protein, 1, 2) == 0);

  char const str[] = "ATGAAACGCATTAGCACCACCATTACCACCAC";
  struct imm_seq seq = imm_seq_unsafe(imm_str(str), &nuclt->super);

  protein_reset(&protein, imm_seq_size(&seq), true, false);

  aye(imm_eseq_setup(&eseq, &seq) == 0);

  char name[IMM_STATE_NAME_SIZE];

  aye(imm_eseq_setup(&eseq, &seq) == 0);

  struct viterbi *viterbi = viterbi_new();
  aye(viterbi_setup(viterbi, protein.core_size) == 0);
  aye(protein_setup_viterbi(&protein, viterbi) == 0);
  aye(near(viterbi_null(viterbi, imm_eseq_size(&eseq), code_fn, (void *)&eseq), 48.9272687711));
  aye(near(viterbi_cost(viterbi, imm_eseq_size(&eseq), code_fn, (void *)&eseq), 54.35543421312));
  aye(viterbi_path(viterbi, imm_eseq_size(&eseq), code_fn, (void *)&eseq) == 0);
  imm_path_reset(&path);
  aye(trellis_unzip(viterbi_trellis(viterbi), imm_eseq_size(&eseq), &path) == 0);

  aye(imm_path_nsteps(&path) == 14);

  aye(imm_path_step(&path, 0)->seqsize == 0);
  aye(imm_path_step(&path, 0)->state_id == STATE_S);
  state_name(imm_path_step(&path, 0)->state_id, name);
  aye(strcmp(name, "S") == 0);


  aye(imm_path_step(&path, 13)->seqsize == 0);
  aye(imm_path_step(&path, 13)->state_id == STATE_T);
  state_name(imm_path_step(&path, 13)->state_id, name);
  aye(strcmp(name, "T") == 0);

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
    aye(codons[i].a == codon.a);
    aye(codons[i].b == codon.b);
    aye(codons[i].c == codon.c);
    ++i;
  }
  aye(rc == 0);
  aye(i == 10);

  imm_eseq_cleanup(&eseq);
  viterbi_del(viterbi);
  protein_cleanup(&protein);
  imm_path_cleanup(&path);
}
