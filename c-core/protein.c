#include "protein.h"
#include "array_size_field.h"
#include "defer_return.h"
#include "lip/1darray/1darray.h"
#include "lip/file/file.h"
#include "lip/lip.h"
#include "model.h"
#include "protein_background.h"
#include "protein_node.h"
#include "unpack.h"
#include "state.h"
#include "pack.h"
#include "xrealloc.h"
#include "xstrcpy.h"
#include <stdlib.h>
#include <string.h>

void protein_init(struct protein *x, struct model_params params)
{
  x->params = params;

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
  return xstrcpy(x->accession, acc, n) ? 0 : DCP_ELONGACCESSION;
}

void protein_setup(struct protein *x, int seq_size, bool multi_hits,
                   bool hmmer3_compat)
{
  assert(seq_size > 0);

  float L = (float)seq_size;

  float q = 0.0;
  float log_q = IMM_LPROB_ZERO;

  if (multi_hits)
  {
    q = 0.5;
    log_q = log(0.5);
  }

  float lp = log(L) - log(L + 2 + q / (1 - q));
  float l1p = log(2 + q / (1 - q)) - log(L + 2 + q / (1 - q));
  float lr = log(L) - log(L + 1);

  struct xtrans t = {0};

  t.NN = t.CC = t.JJ = lp;
  t.NB = t.CT = t.JB = l1p;
  t.RR = lr;
  t.EJ = log_q;
  t.EC = log(1 - q);

  if (hmmer3_compat)
  {
    t.NN = t.CC = t.JJ = log(1);
  }

  protein_null_setup(&x->null, &t);
  x->xtrans = t;
}

int protein_absorb(struct protein *x, struct model *m)
{
  int rc = 0;

  x->params.gencode = m->params.gencode;

  if (x->params.amino != m->params.amino) defer_return(DCP_EDIFFABC);
  if (x->params.code->nuclt != m->params.code->nuclt)
    defer_return(DCP_EDIFFABC);

  size_t n = array_size_field(struct protein, consensus);
  if (!xstrcpy(x->consensus, m->consensus, n)) defer_return(DCP_ELONGCONSENSUS);

  int core_size = x->core_size = m->core_size;
  if (core_size > DCP_CORE_SIZE) defer_return(DCP_ELARGECORESIZE);
  protein_null_absorb(&x->null, &x->score_table, &m->null.nuclt_dist,
                      &m->null.state.super);
  protein_background_absorb(&x->bg, m, &x->score_table);

  x->nodes = xrealloc(x->nodes, (core_size + 1) * sizeof(*x->nodes));
  if (!x->nodes) defer_return(DCP_ENOMEM);

  n = (core_size + 1) * sizeof(*x->emission) * PROTEIN_NODE_SIZE;
  x->emission = xrealloc(x->emission, n);
  if (!x->emission) defer_return(DCP_ENOMEM);

  for (int i = 0; i <= core_size; ++i)
  {
    float *e = x->emission + i * PROTEIN_NODE_SIZE;
    struct model_node const *node = &m->alt.nodes[imm_min(i, core_size - 1)];
    imm_score_table_scores(&x->score_table, &node->match.state.super, e);
    x->nodes[i].nuclt_dist = node->match.nucltd;
    x->nodes[i].trans = m->alt.trans[imm_min(i + 1, core_size)];
    x->nodes[i].emission = e;
  }

  x->BMk = xrealloc(x->BMk, core_size * sizeof(*x->BMk));
  if (!x->BMk && x->core_size > 0) defer_return(DCP_ENOMEM);
  memcpy(x->BMk, m->BMk, x->core_size * sizeof(*x->BMk));

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
  if (!x->params.gencode) return DCP_ESETGENCODE;

  x->core_size = core_size;
  struct imm_rnd rnd = imm_rnd(seed);

  float lprobs[IMM_AMINO_SIZE] = {0};

  imm_lprob_sample(&rnd, IMM_AMINO_SIZE, lprobs);
  imm_lprob_normalize(IMM_AMINO_SIZE, lprobs);

  struct model model = {0};
  model_init(&model, x->params, lprobs);

  int rc = 0;

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
    imm_score_table_cleanup(&x->score_table);
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

int protein_pack(struct protein const *x, struct lip_file *file)
{
  int rc = 0;

  if ((rc = pack_mapsize(file, 10))) return rc;

  if ((rc = pack_key(file, "accession"))) return rc;
  if (!lip_write_cstr(file, x->accession)) return DCP_EFWRITE;

  if ((rc = pack_key(file, "gencode"))) return rc;
  if ((rc = pack_int(file, x->params.gencode->id))) return DCP_EFWRITE;

  if ((rc = pack_key(file, "consensus"))) return rc;
  if (!lip_write_cstr(file, x->consensus)) return DCP_EFWRITE;

  if ((rc = pack_key(file, "core_size"))) return rc;
  if ((rc = pack_int(file, x->core_size))) return DCP_EFWRITE;

  if ((rc = pack_key(file, "null_nuclt_dist"))) return rc;
  if ((rc = nuclt_dist_pack(&x->null.nuclt_dist, file))) return rc;

  if ((rc = pack_key(file, "null_emission"))) return rc;
  if ((rc = pack_f32array(file, PROTEIN_NODE_SIZE, x->null.emission)))
    return rc;

  if ((rc = pack_key(file, "bg_nuclt_dist"))) return rc;
  if ((rc = nuclt_dist_pack(&x->bg.nuclt_dist, file))) return rc;

  if ((rc = pack_key(file, "bg_emission"))) return rc;
  if ((rc = pack_f32array(file, PROTEIN_NODE_SIZE, x->bg.emission))) return rc;

  if ((rc = pack_key(file, "nodes"))) return rc;
  if ((rc = pack_mapsize(file, (x->core_size + 1) * 3))) return rc;
  for (int i = 0; i < x->core_size + 1; ++i)
  {
    if ((rc = pack_key(file, "nuclt_dist"))) return rc;
    if ((rc = nuclt_dist_pack(&x->nodes[i].nuclt_dist, file))) return rc;

    if ((rc = pack_key(file, "trans"))) return rc;
    if ((rc = pack_f32array(file, TRANS_SIZE, x->nodes[i].trans.data)))
      return rc;

    if ((rc = pack_key(file, "emission"))) return rc;
    if ((rc = pack_f32array(file, PROTEIN_NODE_SIZE, x->nodes[i].emission)))
      return rc;
  }
  if ((rc = pack_key(file, "BMk"))) return rc;
  if ((rc = pack_f32array(file, x->core_size, x->BMk))) return rc;

  return 0;
}

int protein_unpack(struct protein *x, struct lip_file *file)
{
  int const accession_size = array_size_field(struct protein, accession);
  int const consensus_size = array_size_field(struct protein, consensus);

  int rc = 0;

  if ((rc = unpack_mapsize(file, 10))) return rc;

  if ((rc = unpack_key(file, "accession"))) return rc;
  if ((rc = read_str(file, accession_size, x->accession))) return rc;

  int gencode_id = 0;
  if ((rc = unpack_key(file, "gencode"))) return rc;
  if ((rc = unpack_int(file, &gencode_id))) return rc;
  if (!(x->params.gencode = imm_gencode_get(gencode_id))) return DCP_EFREAD;

  if ((rc = unpack_key(file, "consensus"))) return rc;
  if ((rc = read_str(file, consensus_size, x->consensus))) return rc;

  if ((rc = unpack_key(file, "core_size"))) return rc;
  if ((rc = unpack_int(file, &x->core_size))) return rc;

  if ((rc = unpack_key(file, "null_nuclt_dist"))) return rc;
  if ((rc = nuclt_dist_unpack(&x->null.nuclt_dist, file))) return rc;

  if ((rc = unpack_key(file, "null_emission"))) return rc;
  if ((rc = unpack_f32array(file, PROTEIN_NODE_SIZE, x->null.emission)))
    return rc;

  if ((rc = unpack_key(file, "bg_nuclt_dist"))) return rc;
  if ((rc = nuclt_dist_unpack(&x->bg.nuclt_dist, file))) return rc;

  if ((rc = unpack_key(file, "bg_emission"))) return rc;
  if ((rc = unpack_f32array(file, PROTEIN_NODE_SIZE, x->bg.emission))) return rc;

  x->nodes = xrealloc(x->nodes, (x->core_size + 1) * sizeof(*x->nodes));
  if (!x->nodes) return DCP_ENOMEM;

  size_t n = (x->core_size + 1) * sizeof(*x->emission) * PROTEIN_NODE_SIZE;
  x->emission = xrealloc(x->emission, n);
  if (!x->emission) return DCP_EFWRITE;

  if ((rc = unpack_key(file, "nodes"))) return rc;
  if ((rc = unpack_mapsize(file, (x->core_size + 1) * 3))) return rc;
  for (int i = 0; i < x->core_size + 1; ++i)
  {
    if ((rc = unpack_key(file, "nuclt_dist"))) return rc;
    if ((rc = nuclt_dist_unpack(&x->nodes[i].nuclt_dist, file))) return rc;

    if ((rc = unpack_key(file, "trans"))) return rc;
    if ((rc = unpack_f32array(file, TRANS_SIZE, x->nodes[i].trans.data)))
      return rc;

    x->nodes[i].emission = x->emission + i * PROTEIN_NODE_SIZE;
    if ((rc = unpack_key(file, "emission"))) return rc;
    if ((rc = unpack_f32array(file, PROTEIN_NODE_SIZE, x->nodes[i].emission)))
      return rc;
  }

  x->BMk = xrealloc(x->BMk, x->core_size * sizeof(*x->BMk));
  if (!x->BMk && x->core_size > 0) return DCP_EFWRITE;

  if ((rc = unpack_key(file, "BMk"))) return rc;
  if ((rc = unpack_f32array(file, x->core_size, x->BMk))) return rc;

  return 0;
}

int protein_decode(struct protein const *x, struct imm_seq const *seq,
                   int state_id, struct imm_codon *codon)
{
  if (state_is_mute(state_id)) return DCP_EINVALSTATE;

  struct nuclt_dist const *nucltd = NULL;
  if (state_is_insert(state_id))
    nucltd = &x->bg.nuclt_dist;
  else if (state_is_match(state_id))
    nucltd = &x->nodes[state_core_idx(state_id)].nuclt_dist;
  else
    nucltd = &x->null.nuclt_dist;

  struct imm_frame_cond cond = {imm_frame_epsilon(x->params.epsilon),
                                &nucltd->nucltp, &nucltd->codonm};

  if (imm_lprob_is_nan(imm_frame_cond_decode(&cond, seq, codon)))
    return DCP_EDECODON;

  return 0;
}
