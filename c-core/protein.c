#include "protein.h"
#include "array_size_field.h"
#include "defer_return.h"
#include "expect.h"
#include "lip/1darray/1darray.h"
#include "lip/file/file.h"
#include "lip/lip.h"
#include "model.h"
#include "protein_background.h"
#include "read.h"
#include "state.h"
#include "strlcpy.h"
#include <stdlib.h>
#include <string.h>

void protein_init(struct dcp_protein *x, struct dcp_model_params params)
{
  x->params = params;

  memset(x->accession, 0, array_size_field(struct dcp_protein, accession));
  x->state_name = &dcp_state_name;

  x->epsilon_frame = imm_frame_epsilon(params.epsilon);

  imm_score_table_init(&x->score_table, &params.code->super);
  memset(x->consensus, 0, array_size_field(struct dcp_protein, consensus));

  x->start_lprob = IMM_LPROB_ONE;
  x->core_size = 0;
  protein_null_init(&x->null);
  dcp_protein_background_init(&x->bg);
  x->nodes = NULL;
  x->nodes_emission = NULL;
  dcp_xtrans_init(&x->xtrans);
  x->BMk = NULL;
}

int protein_set_accession(struct dcp_protein *x, char const *acc)
{
  size_t n = array_size_field(struct dcp_protein, accession);
  return dcp_strlcpy(x->accession, acc, n) < n ? 0 : DCP_ELONGACC;
}

void protein_setup(struct dcp_protein *x, unsigned seq_size, bool multi_hits,
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

  struct dcp_xtrans t = {0};

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

#define MIN(a, b) ((a) < (b) ? (a) : (b))

int protein_absorb(struct dcp_protein *x, struct dcp_model *m)
{
  int rc = 0;

  x->params.gencode = m->params.gencode;

  if (x->params.amino != m->params.amino) defer_return(DCP_EDIFFABC);
  if (x->params.code->nuclt != m->params.code->nuclt)
    defer_return(DCP_EDIFFABC);

  size_t n = array_size_field(struct dcp_protein, consensus);
  dcp_strlcpy(x->consensus, m->consensus, n);

  x->start_lprob = IMM_LPROB_ONE;
  unsigned core_size = x->core_size = m->core_size;
  protein_null_absorb(&x->null, &x->score_table, &m->null.nuclt_dist,
                      &m->null.state.super);
  dcp_protein_background_absorb(&x->bg, m, &x->score_table);

  void *ptr = realloc(x->nodes, (core_size + 1) * sizeof(*x->nodes));
  if (!ptr) defer_return(DCP_ENOMEM);
  x->nodes = ptr;

  ptr =
      realloc(x->nodes_emission,
              (core_size + 1) * sizeof(*x->nodes_emission) * PROTEIN_NODE_SIZE);
  if (!ptr) defer_return(DCP_ENOMEM);
  x->nodes_emission = ptr;

  for (unsigned i = 0; i <= core_size; ++i)
  {
    float *e = x->nodes_emission + i * PROTEIN_NODE_SIZE;
    struct dcp_model_node const *n = &m->alt.nodes[MIN(i, core_size - 1)];
    imm_score_table_scores(&x->score_table, &n->match.state.super, e);
    x->nodes[i].nuclt_dist = n->match.nucltd;
    x->nodes[i].trans = m->alt.trans[MIN(i + 1, core_size)];
    x->nodes[i].emission = e;
  }

  ptr = realloc(x->BMk, core_size * sizeof(*x->BMk));
  if (!ptr && x->core_size > 0) defer_return(DCP_ENOMEM);
  x->BMk = ptr;
  memcpy(x->BMk, m->BMk, x->core_size * sizeof(*x->BMk));

  return rc;

defer:
  if (x->nodes) free(x->nodes);
  if (x->nodes_emission) free(x->nodes_emission);
  if (x->BMk) free(x->BMk);
  x->nodes = NULL;
  x->nodes_emission = NULL;
  x->BMk = NULL;
  return rc;
}

int protein_sample(struct dcp_protein *x, unsigned seed, unsigned core_size)
{
  assert(core_size >= 2);
  if (!x->params.gencode) return DCP_ESETGENCODE;

  x->core_size = core_size;
  struct imm_rnd rnd = imm_rnd(seed);

  float lprobs[IMM_AMINO_SIZE] = {0};

  imm_lprob_sample(&rnd, IMM_AMINO_SIZE, lprobs);
  imm_lprob_normalize(IMM_AMINO_SIZE, lprobs);

  struct dcp_model model = {0};
  dcp_model_init(&model, x->params, lprobs);

  int rc = 0;

  if ((rc = dcp_model_setup(&model, core_size))) goto cleanup;

  for (unsigned i = 0; i < core_size; ++i)
  {
    imm_lprob_sample(&rnd, IMM_AMINO_SIZE, lprobs);
    imm_lprob_normalize(IMM_AMINO_SIZE, lprobs);
    if ((rc = dcp_model_add_node(&model, lprobs, '-'))) goto cleanup;
  }

  for (unsigned i = 0; i < core_size + 1; ++i)
  {
    struct dcp_trans t = {0};
    imm_lprob_sample(&rnd, TRANS_SIZE, t.data);
    if (i == 0) t.DD = IMM_LPROB_ZERO;
    if (i == core_size)
    {
      t.MD = IMM_LPROB_ZERO;
      t.DD = IMM_LPROB_ZERO;
    }
    imm_lprob_normalize(TRANS_SIZE, t.data);
    if ((rc = dcp_model_add_trans(&model, t))) goto cleanup;
  }

  rc = protein_absorb(x, &model);

cleanup:
  dcp_model_cleanup(&model);
  return rc;
}

void protein_cleanup(struct dcp_protein *x)
{
  if (x)
  {
    imm_score_table_cleanup(&x->score_table);
    if (x->nodes) free(x->nodes);
    if (x->nodes_emission) free(x->nodes_emission);
    if (x->BMk) free(x->BMk);
    x->nodes = NULL;
    x->nodes_emission = NULL;
    x->BMk = NULL;
  }
}

void protein_dump(struct dcp_protein const *x, FILE *restrict fp)
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
  for (unsigned i = 0; i < x->core_size + 1; ++i)
  {
    fprintf(fp, "\n");
    fprintf(fp, "### nodes[%d]\n", i);

    fprintf(fp, "nuclt_dist: ");
    dcp_nuclt_dist_dump(&x->nodes[i].nuclt_dist, fp);
    fputc('\n', fp);

    fprintf(fp, "MM  MI  MD  IM  II  DM  DD\n");
    imm_dump_array_f32(TRANS_SIZE, x->nodes[i].trans.data, fp);
    fputc('\n', fp);

    fprintf(fp, "emis: ");
    imm_dump_array_f32(PROTEIN_NODE_SIZE, x->nodes[i].emission, fp);
    fputc('\n', fp);
  }

  fprintf(fp, "xtrans: ");
  dcp_xtrans_dump(&x->xtrans, fp);
  fputc('\n', fp);

  fputc('\n', fp);
  fprintf(fp, "BMk: ");
  imm_dump_array_f32(x->core_size, x->BMk, fp);
  fputc('\n', fp);
  fputc('\n', fp);
}

int protein_pack(struct dcp_protein const *x, struct lip_file *file)
{
  int rc = 0;

  if (!lip_write_map_size(file, 10)) return DCP_EFWRITE;

  if (!lip_write_cstr(file, "accession")) return DCP_EFWRITE;
  if (!lip_write_cstr(file, x->accession)) return DCP_EFWRITE;

  if (!lip_write_cstr(file, "gencode")) return DCP_EFWRITE;
  if (!lip_write_int(file, x->params.gencode->id)) return DCP_EFWRITE;

  if (!lip_write_cstr(file, "consensus")) return DCP_EFWRITE;
  if (!lip_write_cstr(file, x->consensus)) return DCP_EFWRITE;

  if (!lip_write_cstr(file, "core_size")) return DCP_EFWRITE;
  if (!lip_write_int(file, x->core_size)) return DCP_EFWRITE;

  if (!lip_write_cstr(file, "null_nuclt_dist")) return DCP_EFWRITE;
  if ((rc = dcp_nuclt_dist_pack(&x->null.nuclt_dist, file))) return rc;

  if (!lip_write_cstr(file, "null_emission")) return DCP_EFWRITE;
  if (!lip_write_1darray_size_type(file, PROTEIN_NODE_SIZE, LIP_1DARRAY_F32))
    return DCP_EFWRITE;
  if (!lip_write_1darray_float_data(file, PROTEIN_NODE_SIZE, x->null.emission))
    return DCP_EFWRITE;

  if (!lip_write_cstr(file, "bg_nuclt_dist")) return DCP_EFWRITE;
  if ((rc = dcp_nuclt_dist_pack(&x->bg.nuclt_dist, file))) return rc;

  if (!lip_write_cstr(file, "bg_emission")) return DCP_EFWRITE;
  if (!lip_write_1darray_size_type(file, PROTEIN_NODE_SIZE, LIP_1DARRAY_F32))
    return DCP_EFWRITE;
  if (!lip_write_1darray_float_data(file, PROTEIN_NODE_SIZE, x->bg.emission))
    return DCP_EFWRITE;

  if (!lip_write_cstr(file, "nodes")) return DCP_EFWRITE;
  if (!lip_write_map_size(file, (x->core_size + 1) * 3)) return DCP_EFWRITE;
  for (unsigned i = 0; i < x->core_size + 1; ++i)
  {
    if (!lip_write_cstr(file, "nuclt_dist")) return DCP_EFWRITE;
    if ((rc = dcp_nuclt_dist_pack(&x->nodes[i].nuclt_dist, file))) return rc;

    if (!lip_write_cstr(file, "trans")) return DCP_EFWRITE;
    if (!lip_write_1darray_size_type(file, TRANS_SIZE, LIP_1DARRAY_F32))
      return DCP_EFWRITE;
    if (!lip_write_1darray_float_data(file, TRANS_SIZE, x->nodes[i].trans.data))
      return DCP_EFWRITE;

    if (!lip_write_cstr(file, "emission")) return DCP_EFWRITE;
    if (!lip_write_1darray_size_type(file, PROTEIN_NODE_SIZE, LIP_1DARRAY_F32))
      return DCP_EFWRITE;
    if (!lip_write_1darray_float_data(file, PROTEIN_NODE_SIZE,
                                      x->nodes[i].emission))
      return DCP_EFWRITE;
  }

  if (!lip_write_cstr(file, "BMk")) return DCP_EFWRITE;
  if (!lip_write_1darray_size_type(file, x->core_size, LIP_1DARRAY_F32))
    return DCP_EFWRITE;
  if (!lip_write_1darray_float_data(file, x->core_size, x->BMk))
    return DCP_EFWRITE;

  return 0;
}

int protein_unpack(struct dcp_protein *x, struct lip_file *file)
{
  unsigned const accession_size =
      array_size_field(struct dcp_protein, accession);
  unsigned const consensus_size =
      array_size_field(struct dcp_protein, consensus);

  int rc = 0;

  if ((rc = read_mapsize(file, 10))) return rc;

  if ((rc = read_key(file, "accession"))) return rc;
  if ((rc = read_str(file, accession_size, x->accession))) return rc;

  unsigned gencode_id = 0;
  if ((rc = read_key(file, "gencode"))) return rc;
  if ((rc = read_int(file, &gencode_id))) return rc;
  if (!(x->params.gencode = imm_gencode_get(gencode_id))) return DCP_EFREAD;

  if ((rc = read_key(file, "consensus"))) return rc;
  if ((rc = read_str(file, consensus_size, x->consensus))) return rc;

  if ((rc = read_key(file, "core_size"))) return rc;
  if ((rc = read_int(file, &x->core_size))) return rc;

  if ((rc = read_key(file, "null_nuclt_dist"))) return rc;
  if ((rc = dcp_nuclt_dist_unpack(&x->null.nuclt_dist, file))) return rc;

  if ((rc = read_key(file, "null_emission"))) return rc;
  if ((rc = read_f32array(file, PROTEIN_NODE_SIZE, x->null.emission)))
    return rc;

  if ((rc = read_key(file, "bg_nuclt_dist"))) return rc;
  if ((rc = dcp_nuclt_dist_unpack(&x->bg.nuclt_dist, file))) return rc;

  if ((rc = read_key(file, "bg_emission"))) return rc;
  if ((rc = read_f32array(file, PROTEIN_NODE_SIZE, x->bg.emission))) return rc;

  void *ptr = realloc(x->nodes, (x->core_size + 1) * sizeof(*x->nodes));
  if (!ptr) return DCP_ENOMEM;
  x->nodes = ptr;

  ptr = realloc(x->nodes_emission, (x->core_size + 1) *
                                       sizeof(*x->nodes_emission) *
                                       PROTEIN_NODE_SIZE);
  if (!ptr) return DCP_EFWRITE;
  x->nodes_emission = ptr;

  if ((rc = read_key(file, "nodes"))) return rc;
  if ((rc = read_mapsize(file, (x->core_size + 1) * 3))) return rc;
  for (unsigned i = 0; i < x->core_size + 1; ++i)
  {
    if ((rc = read_key(file, "nuclt_dist"))) return rc;
    if ((rc = dcp_nuclt_dist_unpack(&x->nodes[i].nuclt_dist, file))) return rc;

    if ((rc = read_key(file, "trans"))) return rc;
    if ((rc = read_f32array(file, TRANS_SIZE, x->nodes[i].trans.data)))
      return rc;

    float *emission = x->nodes_emission + i * PROTEIN_NODE_SIZE;
    x->nodes[i].emission = emission;
    if ((rc = read_key(file, "emission"))) return rc;
    if ((rc = read_f32array(file, PROTEIN_NODE_SIZE, x->nodes[i].emission)))
      return rc;
  }

  ptr = realloc(x->BMk, x->core_size * sizeof(*x->BMk));
  if (!ptr && x->core_size > 0) return DCP_EFWRITE;
  x->BMk = ptr;

  if ((rc = read_key(file, "BMk"))) return rc;
  if ((rc = read_f32array(file, x->core_size, x->BMk))) return rc;

  return 0;
}

int protein_decode(struct dcp_protein const *x, struct imm_seq const *seq,
                   unsigned state_id, struct imm_codon *codon)
{
  assert(!dcp_state_is_mute(state_id));

  struct dcp_nuclt_dist const *nucltd = NULL;
  if (dcp_state_is_insert(state_id))
  {
    nucltd = &x->bg.nuclt_dist;
  }
  else if (dcp_state_is_match(state_id))
  {
    unsigned idx = dcp_state_idx(state_id);
    nucltd = &x->nodes[idx].nuclt_dist;
  }
  else
    nucltd = &x->null.nuclt_dist;

  struct imm_frame_cond cond = {x->epsilon_frame, &nucltd->nucltp,
                                &nucltd->codonm};

  if (imm_lprob_is_nan(imm_frame_cond_decode(&cond, seq, codon)))
    return DCP_EDECODON;

  return 0;
}
