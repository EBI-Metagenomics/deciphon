#include "protein.h"
#include "array_size_field.h"
#include "defer_return.h"
#include "error.h"
#include "expect.h"
#include "imm_dump.h"
#include "imm_frame_cond.h"
#include "imm_gencode.h"
#include "imm_lprob.h"
#include "imm_nuclt_code.h"
#include "imm_rnd.h"
#include "lio.h"
#include "min.h"
#include "model.h"
#include "protein_background.h"
#include "protein_node.h"
#include "read.h"
#include "state.h"
#include "viterbi.h"
#include "write.h"
#include "xrealloc.h"
#include "xstrcpy.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void protein_init(struct protein *x)
{
  memset(x, 0, sizeof(*x));
  x->params.code = NULL;
  x->has_ga = false;
  x->nodes = NULL;
  x->emission = NULL;
  x->BMk = NULL;
}

void protein_setup(struct protein *x, struct model_params params, bool multi_hits, bool hmmer3_compat)
{
  x->params = params;
  x->multi_hits = multi_hits;
  x->hmmer3_compat = hmmer3_compat;

  memset(x->accession, 0, array_size_field(struct protein, accession));
  imm_score_table_init(&x->score_table, &params.code->super);
  memset(x->consensus, 0, array_size_field(struct protein, consensus));

  x->core_size = 0;
  protein_null_init(&x->null, params.code->nuclt);
  protein_background_init(&x->bg, params.code->nuclt);
  x->nodes = NULL;
  x->emission = NULL;
  xtrans_init(&x->xtrans);
  x->BMk = NULL;
}

int protein_set_accession(struct protein *x, char const *acc)
{
  size_t n = array_size_field(struct protein, accession);
  return xstrcpy(x->accession, acc, n) ? error(DCP_ELONGACCESSION) : 0;
}

void protein_reset(struct protein *x, int seq_size)
{
  xtrans_setup(&x->xtrans, x->multi_hits, x->hmmer3_compat, seq_size);
  protein_null_setup(&x->null, x->xtrans.RR);
}

int protein_absorb(struct protein *x, struct model *m)
{
  int rc = 0;

  x->params.gencode = m->params.gencode;
  x->has_ga = m->has_ga;

  if (x->params.amino != m->params.amino) defer_return(error(DCP_EDIFFABC));
  if (x->params.code->nuclt != m->params.code->nuclt)
    defer_return(error(DCP_EDIFFABC));

  size_t n = array_size_field(struct protein, consensus);
  if (xstrcpy(x->consensus, m->consensus, n))
    defer_return(error(DCP_ELONGCONSENSUS));

  int core_size = x->core_size = m->core_size;
  if (core_size > MODEL_MAX) defer_return(error(DCP_ELARGECORESIZE));
  protein_null_absorb(&x->null, &x->score_table, &m->null.nuclt_dist,
                      &m->null.state.super);
  protein_background_absorb(&x->bg, m, &x->score_table);

  x->nodes = xrealloc(x->nodes, (core_size + 1) * sizeof(*x->nodes));
  if (!x->nodes) defer_return(error(DCP_ENOMEM));

  n = (core_size + 1) * sizeof(*x->emission) * PROTEIN_NODE_SIZE;
  x->emission = xrealloc(x->emission, n);
  if (!x->emission) defer_return(error(DCP_ENOMEM));

#pragma omp parallel for
  for (int i = 0; i <= core_size; ++i)
  {
    float *e = x->emission + i * PROTEIN_NODE_SIZE;
    struct model_node const *node = &m->alt.nodes[min(i, core_size - 1)];
    imm_score_table_scores(&x->score_table, &node->match.state.super, e);
    x->nodes[i].nuclt_dist = node->match.nucltd;
    x->nodes[i].trans = m->alt.trans[min(i + 1, core_size)];
    x->nodes[i].emission = e;
  }

  x->BMk = xrealloc(x->BMk, core_size * sizeof(*x->BMk));
  if (!x->BMk && x->core_size > 0) defer_return(error(DCP_ENOMEM));
  if (x->BMk) memcpy(x->BMk, m->BMk, x->core_size * sizeof(*x->BMk));

  return rc;

defer:
  free(x->nodes);
  free(x->emission);
  free(x->BMk);
  x->nodes = NULL;
  x->emission = NULL;
  x->BMk = NULL;
  return rc;
}

int protein_sample(struct protein *x, int seed, int core_size)
{
  assert(core_size >= 2);
  if (!x->params.gencode) return error(DCP_ESETGENCODE);

  x->core_size = core_size;
  struct imm_rnd rnd = imm_rnd(seed);

  float lprobs[IMM_AMINO_SIZE] = {0};

  imm_lprob_sample(&rnd, IMM_AMINO_SIZE, lprobs);
  imm_lprob_normalize(IMM_AMINO_SIZE, lprobs);

  struct model model = {0};
  int rc = model_init(&model, x->params, lprobs);
  if (rc) return rc;

  if ((rc = model_setup(&model, core_size))) goto cleanup;

  for (int i = 0; i < core_size; ++i)
  {
    imm_lprob_sample(&rnd, IMM_AMINO_SIZE, lprobs);
    imm_lprob_normalize(IMM_AMINO_SIZE, lprobs);
    if ((rc = model_add_node(&model, lprobs, '-'))) goto cleanup;
  }

  for (int i = 0; i < core_size + 1; ++i)
  {
    struct trans t = {0};
    imm_lprob_sample(&rnd, TRANS_SIZE, t.data);
    if (i == 0) t.DD = IMM_LPROB_ZERO;
    if (i == core_size)
    {
      t.MD = IMM_LPROB_ZERO;
      t.DD = IMM_LPROB_ZERO;
    }
    imm_lprob_normalize(TRANS_SIZE, t.data);
    if ((rc = model_add_trans(&model, t))) goto cleanup;
  }

  rc = protein_absorb(x, &model);

cleanup:
  model_cleanup(&model);
  return rc;
}

void protein_cleanup(struct protein *x)
{
  if (x)
  {
    if (x->params.code) imm_score_table_cleanup(&x->score_table);
    x->params.code = NULL;
    free(x->nodes);
    free(x->emission);
    free(x->BMk);
    x->nodes = NULL;
    x->emission = NULL;
    x->BMk = NULL;
  }
}

void protein_dump(struct protein const *x, FILE *fp)
{
  fprintf(fp, "# protein\n");
  fprintf(fp, "entry_dist: %d\n", x->params.entry_dist);
  fprintf(fp, "epsilon: %f\n", x->params.epsilon);
  fprintf(fp, "core_size: %d\n", x->core_size);

  fprintf(fp, "## null\n");
  fprintf(fp, "FR: %f\n", 0.);
  fprintf(fp, "RR: %f\n", x->null.RR);
  fprintf(fp, "RG: %f\n", 0.);
  fprintf(fp, "emis: ");
  imm_dump_array_f32(PROTEIN_NODE_SIZE, x->null.emission, fp);
  fprintf(fp, "\n\n");

  fprintf(fp, "## bg\n");
  fprintf(fp, "emis: ");
  imm_dump_array_f32(PROTEIN_NODE_SIZE, x->bg.emission, fp);
  fprintf(fp, "\n\n");

  fprintf(fp, "## nodes\n");
  for (int i = 0; i < x->core_size + 1; ++i)
  {
    fprintf(fp, "\n");
    fprintf(fp, "### nodes[%d]\n", i);

    fprintf(fp, "nuclt_dist: ");
    nuclt_dist_dump(&x->nodes[i].nuclt_dist, fp);
    fputc('\n', fp);

    fprintf(fp, "MM  MI  MD  IM  II  DM  DD\n");
    imm_dump_array_f32(TRANS_SIZE, x->nodes[i].trans.data, fp);
    fputc('\n', fp);

    fprintf(fp, "emis: ");
    imm_dump_array_f32(PROTEIN_NODE_SIZE, x->nodes[i].emission, fp);
    fputc('\n', fp);
  }

  fprintf(fp, "xtrans: ");
  xtrans_dump(&x->xtrans, fp);
  fputc('\n', fp);

  fputc('\n', fp);
  fprintf(fp, "BMk: ");
  imm_dump_array_f32(x->core_size, x->BMk, fp);
  fputc('\n', fp);
  fputc('\n', fp);
}

int protein_pack(struct protein const *x, struct lio_writer *file)
{
  int rc = 0;

  if ((rc = write_map(file, 10))) return rc;

  if ((rc = write_cstring(file, "accession"))) return rc;
  if ((rc = write_cstring(file, x->accession))) return rc;

  if ((rc = write_cstring(file, "gencode"))) return rc;
  if ((rc = write_int(file, x->params.gencode->id))) return error(rc);

  if ((rc = write_cstring(file, "consensus"))) return rc;
  if ((rc = write_cstring(file, x->consensus))) return rc;

  if ((rc = write_cstring(file, "core_size"))) return rc;
  if ((rc = write_int(file, x->core_size))) return error(rc);

  if ((rc = write_cstring(file, "null_nuclt_dist"))) return rc;
  if ((rc = nuclt_dist_pack(&x->null.nuclt_dist, file))) return rc;

  if ((rc = write_cstring(file, "null_emission"))) return rc;
  if ((rc = write_f32array(file, PROTEIN_NODE_SIZE, x->null.emission)))
    return rc;

  if ((rc = write_cstring(file, "bg_nuclt_dist"))) return rc;
  if ((rc = nuclt_dist_pack(&x->bg.nuclt_dist, file))) return rc;

  if ((rc = write_cstring(file, "bg_emission"))) return rc;
  if ((rc = write_f32array(file, PROTEIN_NODE_SIZE, x->bg.emission))) return rc;

  if ((rc = write_cstring(file, "nodes"))) return rc;
  if ((rc = write_map(file, (x->core_size + 1) * 3))) return rc;
  for (int i = 0; i < x->core_size + 1; ++i)
  {
    if ((rc = write_cstring(file, "nuclt_dist"))) return rc;
    if ((rc = nuclt_dist_pack(&x->nodes[i].nuclt_dist, file))) return rc;

    if ((rc = write_cstring(file, "trans"))) return rc;
    if ((rc = write_f32array(file, TRANS_SIZE, x->nodes[i].trans.data)))
      return rc;

    if ((rc = write_cstring(file, "emission"))) return rc;
    if ((rc = write_f32array(file, PROTEIN_NODE_SIZE, x->nodes[i].emission)))
      return rc;
  }
  if ((rc = write_cstring(file, "BMk"))) return rc;
  if ((rc = write_f32array(file, x->core_size, x->BMk))) return rc;

  return 0;
}

int protein_unpack(struct protein *x, struct lio_reader *file)
{
  int const accession_size = array_size_field(struct protein, accession);
  int const consensus_size = array_size_field(struct protein, consensus);

  int rc = 0;

  if ((rc = expect_map(file, 10))) return rc;

  if ((rc = expect_key(file, "accession"))) return rc;
  if ((rc = read_cstring(file, accession_size, x->accession))) return rc;

  int gencode_id = 0;
  if ((rc = expect_key(file, "gencode"))) return rc;
  if ((rc = read_int(file, &gencode_id))) return rc;
  if (!(x->params.gencode = imm_gencode_get(gencode_id)))
    return error(DCP_EFREAD);

  if ((rc = expect_key(file, "consensus"))) return rc;
  if ((rc = read_cstring(file, consensus_size, x->consensus))) return rc;

  if ((rc = expect_key(file, "core_size"))) return rc;
  if ((rc = read_int(file, &x->core_size))) return rc;

  if ((rc = expect_key(file, "null_nuclt_dist"))) return rc;
  if ((rc = nuclt_dist_unpack(&x->null.nuclt_dist, file))) return rc;

  if ((rc = expect_key(file, "null_emission"))) return rc;
  if ((rc = read_f32array(file, PROTEIN_NODE_SIZE, x->null.emission)))
    return rc;

  if ((rc = expect_key(file, "bg_nuclt_dist"))) return rc;
  if ((rc = nuclt_dist_unpack(&x->bg.nuclt_dist, file))) return rc;

  if ((rc = expect_key(file, "bg_emission"))) return rc;
  if ((rc = read_f32array(file, PROTEIN_NODE_SIZE, x->bg.emission))) return rc;

  x->nodes = xrealloc(x->nodes, (x->core_size + 1) * sizeof(*x->nodes));
  if (!x->nodes) return error(DCP_ENOMEM);

  size_t n = (x->core_size + 1) * sizeof(*x->emission) * PROTEIN_NODE_SIZE;
  x->emission = xrealloc(x->emission, n);
  if (!x->emission) return error(DCP_EFWRITE);

  if ((rc = expect_key(file, "nodes"))) return rc;
  if ((rc = expect_map(file, (x->core_size + 1) * 3))) return rc;
  for (int i = 0; i < x->core_size + 1; ++i)
  {
    if ((rc = expect_key(file, "nuclt_dist"))) return rc;
    if ((rc = nuclt_dist_unpack(&x->nodes[i].nuclt_dist, file))) return rc;

    if ((rc = expect_key(file, "trans"))) return rc;
    if ((rc = read_f32array(file, TRANS_SIZE, x->nodes[i].trans.data)))
      return rc;

    x->nodes[i].emission = x->emission + i * PROTEIN_NODE_SIZE;
    if ((rc = expect_key(file, "emission"))) return rc;
    if ((rc = read_f32array(file, PROTEIN_NODE_SIZE, x->nodes[i].emission)))
      return rc;
  }

  x->BMk = xrealloc(x->BMk, x->core_size * sizeof(*x->BMk));
  if (!x->BMk && x->core_size > 0) return error(DCP_EFWRITE);

  if ((rc = expect_key(file, "BMk"))) return rc;
  if ((rc = read_f32array(file, x->core_size, x->BMk))) return rc;

  return 0;
}

int protein_setup_viterbi(struct protein const *x, struct viterbi *v)
{
  int K = x->core_size;
  int rc = viterbi_setup(v, K);
  if (rc) return rc;

  xtrans_setup_viterbi(&x->xtrans, v);

  for (int k = 0; k < K; ++k)
  {
    viterbi_set_core_trans(v, CORE_TRANS_BM, -x->BMk[k], k);
  }

  viterbi_set_core_trans(v, CORE_TRANS_MM, INFINITY, 0);
  viterbi_set_core_trans(v, CORE_TRANS_MD, INFINITY, 0);
  viterbi_set_core_trans(v, CORE_TRANS_IM, INFINITY, 0);
  viterbi_set_core_trans(v, CORE_TRANS_DM, INFINITY, 0);
  viterbi_set_core_trans(v, CORE_TRANS_DD, INFINITY, 0);
  struct protein_node *nodes = x->nodes;
  for (int k = 0; k < K - 1; ++k)
  {
    viterbi_set_core_trans(v, CORE_TRANS_MM, -nodes[k].trans.MM, k + 1);
    viterbi_set_core_trans(v, CORE_TRANS_MI, -nodes[k].trans.MI, k + 0);
    viterbi_set_core_trans(v, CORE_TRANS_MD, -nodes[k].trans.MD, k + 1);
    viterbi_set_core_trans(v, CORE_TRANS_IM, -nodes[k].trans.IM, k + 1);
    viterbi_set_core_trans(v, CORE_TRANS_II, -nodes[k].trans.II, k + 0);
    viterbi_set_core_trans(v, CORE_TRANS_DM, -nodes[k].trans.DM, k + 1);
    viterbi_set_core_trans(v, CORE_TRANS_DD, -nodes[k].trans.DD, k + 1);
  }
  viterbi_set_core_trans(v, CORE_TRANS_MI, INFINITY, K - 1);
  viterbi_set_core_trans(v, CORE_TRANS_II, INFINITY, K - 1);

  for (int i = 0; i < viterbi_table_size(); ++i)
  {
    viterbi_set_null(v, -x->null.emission[i], i);
    viterbi_set_background(v, -x->bg.emission[i], i);

    for (int k = 0; k < K; ++k)
      viterbi_set_match(v, -x->nodes[k].emission[i], k, i);
  }
  return 0;
}
