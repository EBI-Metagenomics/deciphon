#include "p7.h"
#include "array_size_field.h"
#include "defer_return.h"
#include "expect.h"
#include "lip/1darray/1darray.h"
#include "lip/file/file.h"
#include "lip/lip.h"
#include "model.h"
#include "p7_background.h"
#include "state.h"
#include "strlcpy.h"
#include <stdlib.h>
#include <string.h>

void p7_init(struct p7 *x, struct dcp_model_params params)
{
  x->params = params;

  memset(x->accession, 0, array_size_field(struct p7, accession));
  x->state_name = dcp_state_name;

  x->epsilon_frame = imm_frame_epsilon(params.epsilon);

  imm_score_table_init(&x->score_table, &params.code->super);
  memset(x->consensus, 0, array_size_field(struct p7, consensus));

  x->start_lprob = IMM_LPROB_NAN;
  x->core_size = 0;
  p7_null_init(&x->null);
  p7_background_init(&x->bg);
  x->nodes = NULL;
  dcp_xtrans_init(&x->xtrans);
  x->BMk = NULL;
}

int p7_set_accession(struct p7 *x, char const *acc)
{
  size_t n = array_size_field(struct p7, accession);
  return dcp_strlcpy(x->accession, acc, n) < n ? 0 : DCP_ELONGACC;
}

void p7_setup(struct p7 *x, unsigned seq_size, bool multi_hits,
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

  p7_null_setup(&x->null, &t);
  x->xtrans = t;
}

int p7_absorb(struct p7 *x, struct dcp_model *m)
{
  int rc = 0;

  x->params.gencode = m->params.gencode;

  if (x->params.amino != m->params.amino) defer_return(DCP_EDIFFABC);
  if (x->params.code->nuclt != m->params.code->nuclt)
    defer_return(DCP_EDIFFABC);

  size_t n = array_size_field(struct p7, consensus);
  dcp_strlcpy(x->consensus, m->consensus, n);

  x->start_lprob = IMM_LPROB_ONE;
  unsigned core_size = x->core_size = m->core_size;
  p7_null_absorb(&x->null, &x->score_table, &m->null.nuclt_dist,
                 &m->null.state.super);
  p7_background_absorb(&x->bg, m, &x->score_table);

  void *ptr = realloc(x->nodes, (core_size + 1) * sizeof(*x->nodes));
  if (!ptr) defer_return(DCP_ENOMEM);
  x->nodes = ptr;

  for (unsigned i = 0; i < core_size; ++i)
  {
    p7_node_absorb_emission(x->nodes + i, &m->alt.nodes[i].match.nucltd,
                            &x->score_table,
                            &m->alt.nodes[i].match.state.super);
    p7_node_absorb_transition(x->nodes + i, m->alt.trans + i);
  }
  p7_node_absorb_emission(
      x->nodes + core_size, &m->alt.nodes[core_size - 1].match.nucltd,
      &x->score_table, &m->alt.nodes[core_size - 1].match.state.super);
  p7_node_absorb_transition(x->nodes + core_size, m->alt.trans + core_size);

  ptr = realloc(x->BMk, core_size * sizeof(*x->BMk));
  if (!ptr && x->core_size > 0) defer_return(DCP_ENOMEM);
  x->BMk = ptr;
  memcpy(x->BMk, m->BMk, x->core_size * sizeof(x->BMk));

  return rc;

defer:
  if (x->nodes) free(x->nodes);
  if (x->BMk) free(x->BMk);
  x->nodes = NULL;
  x->BMk = NULL;
  return rc;
}

int p7_sample(struct p7 *x, unsigned seed, unsigned core_size)
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

  rc = p7_absorb(x, &model);

cleanup:
  dcp_model_cleanup(&model);
  return rc;
}

void p7_dump(struct p7 const *x, FILE *restrict fp)
{
  fprintf(fp, "# p7\n");
  fprintf(fp, "entry_dist: %d\n", x->params.entry_dist);
  fprintf(fp, "epsilon: %f\n", x->params.epsilon);
  fprintf(fp, "core_size: %d\n", x->core_size);

  fprintf(fp, "## null\n");
  fprintf(fp, "RR: %f\n", x->null.RR);
  fprintf(fp, "emis: ");
  imm_dump_array_f32(P7_NODE_SIZE, x->null.emission, fp);
  fprintf(fp, "\n\n");

  fprintf(fp, "## bg\n");
  fprintf(fp, "emis: ");
  imm_dump_array_f32(P7_NODE_SIZE, x->bg.emission, fp);
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
    imm_dump_array_f32(P7_NODE_SIZE, x->nodes[i].emission, fp);
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

int p7_pack(struct p7 const *x, struct lip_file *file)
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
  if (!lip_write_1darray_size_type(file, P7_NODE_SIZE, LIP_1DARRAY_F32))
    return DCP_EFWRITE;
  if (!lip_write_1darray_float_data(file, P7_NODE_SIZE, x->null.emission))
    return DCP_EFWRITE;

  if (!lip_write_cstr(file, "bg_nuclt_dist")) return DCP_EFWRITE;
  if ((rc = dcp_nuclt_dist_pack(&x->bg.nuclt_dist, file))) return rc;

  if (!lip_write_cstr(file, "bg_emission")) return DCP_EFWRITE;
  if (!lip_write_1darray_size_type(file, P7_NODE_SIZE, LIP_1DARRAY_F32))
    return DCP_EFWRITE;
  if (!lip_write_1darray_float_data(file, P7_NODE_SIZE, x->bg.emission))
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
    if (!lip_write_1darray_size_type(file, P7_NODE_SIZE, LIP_1DARRAY_F32))
      return DCP_EFWRITE;
    if (!lip_write_1darray_float_data(file, P7_NODE_SIZE, x->nodes[i].emission))
      return DCP_EFWRITE;
  }

  if (!lip_write_cstr(file, "BMk")) return DCP_EFWRITE;
  if (!lip_write_1darray_size_type(file, x->core_size, LIP_1DARRAY_F32))
    return DCP_EFWRITE;
  if (!lip_write_1darray_float_data(file, x->core_size, x->BMk))
    return DCP_EFWRITE;

  return 0;
}

int p7_unpack(struct p7 *x, struct lip_file *file)
{
  unsigned const accession_size = array_size_field(struct p7, accession);
  unsigned const consensus_size = array_size_field(struct p7, consensus);

  int rc = 0;
  unsigned size = 0;
  enum lip_1darray_type type = 0;

  if ((rc = dcp_expect_map_size(file, 10))) return rc;

  if ((rc = dcp_expect_map_key(file, "accession"))) return rc;
  if (!lip_read_cstr(file, accession_size, x->accession)) return DCP_EFREAD;

  unsigned gencode_id = 0;
  if ((rc = dcp_expect_map_key(file, "gencode"))) return rc;
  if (!lip_read_int(file, &gencode_id)) return DCP_EFREAD;
  if (!(x->params.gencode = imm_gencode_get(gencode_id))) return DCP_EFREAD;

  if ((rc = dcp_expect_map_key(file, "consensus"))) return rc;
  if (!lip_read_cstr(file, consensus_size, x->consensus)) return DCP_EFREAD;

  if ((rc = dcp_expect_map_key(file, "core_size"))) return rc;
  if (!lip_read_int(file, &x->core_size)) return DCP_EFREAD;

  if ((rc = dcp_expect_map_key(file, "null_nuclt_dist"))) return rc;
  if ((rc = dcp_nuclt_dist_unpack(&x->null.nuclt_dist, file))) return rc;

  if ((rc = dcp_expect_map_key(file, "null_emission"))) return rc;
  lip_read_1darray_size_type(file, &size, &type);
  if (type != LIP_1DARRAY_F32) return rc;
  if (size != P7_NODE_SIZE) return DCP_EFREAD;
  if (!lip_read_1darray_float_data(file, P7_NODE_SIZE, x->null.emission))
    return DCP_EFWRITE;

  if ((rc = dcp_expect_map_key(file, "bg_nuclt_dist"))) return rc;
  if ((rc = dcp_nuclt_dist_unpack(&x->bg.nuclt_dist, file))) return rc;

  if ((rc = dcp_expect_map_key(file, "bg_emission"))) return rc;
  lip_read_1darray_size_type(file, &size, &type);
  if (type != LIP_1DARRAY_F32) return rc;
  if (size != P7_NODE_SIZE) return DCP_EFREAD;
  if (!lip_read_1darray_float_data(file, P7_NODE_SIZE, x->bg.emission))
    return DCP_EFWRITE;

  void *ptr = realloc(x->nodes, (x->core_size + 1) * sizeof(*x->nodes));
  if (!ptr) return DCP_EFWRITE;
  x->nodes = ptr;

  if ((rc = dcp_expect_map_key(file, "nodes"))) return rc;
  if ((rc = dcp_expect_map_size(file, (x->core_size + 1) * 3))) return rc;
  for (unsigned i = 0; i < x->core_size + 1; ++i)
  {
    if ((rc = dcp_expect_map_key(file, "nuclt_dist"))) return rc;
    if ((rc = dcp_nuclt_dist_unpack(&x->nodes[i].nuclt_dist, file))) return rc;

    if ((rc = dcp_expect_map_key(file, "trans"))) return rc;
    lip_read_1darray_size_type(file, &size, &type);
    if (type != LIP_1DARRAY_F32) return rc;
    if (size != TRANS_SIZE) return DCP_EFREAD;
    if (!lip_read_1darray_float_data(file, TRANS_SIZE, x->nodes[i].trans.data))
      return DCP_EFWRITE;

    if ((rc = dcp_expect_map_key(file, "emission"))) return rc;
    lip_read_1darray_size_type(file, &size, &type);
    if (type != LIP_1DARRAY_F32) return rc;
    if (size != P7_NODE_SIZE) return DCP_EFREAD;
    if (!lip_read_1darray_float_data(file, P7_NODE_SIZE, x->nodes[i].emission))
      return DCP_EFWRITE;
  }

  if ((rc = dcp_expect_map_key(file, "BMk"))) return rc;
  lip_read_1darray_size_type(file, &size, &type);
  if (type != LIP_1DARRAY_F32) return rc;
  if (size != x->core_size) return DCP_EFREAD;

  ptr = realloc(x->BMk, x->core_size * sizeof(*x->BMk));
  if (!ptr && x->core_size > 0) return DCP_EFWRITE;
  x->BMk = ptr;
  if (!lip_read_1darray_float_data(file, x->core_size, x->BMk))
    return DCP_EFWRITE;

  return 0;
}
