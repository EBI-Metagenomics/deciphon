#include "protein.h"
#include "array_size_field.h"
#include "db_reader.h"
#include "expect.h"
#include "lip/file/file.h"
#include "lip/lip.h"
#include "protein.h"
#include "rc.h"
#include "strkcpy.h"
#include <assert.h>
#include <string.h>

void dcp_protein_init(struct dcp_protein *x, struct imm_gencode const *gc,
                      struct imm_amino const *amino,
                      struct imm_nuclt_code const *code,
                      enum entry_dist entry_dist, float epsilon)
{
  x->gencode = gc;
  memset(x->accession, 0, array_size_field(struct dcp_protein, accession));
  x->state_name = dcp_state_name;
  x->imm_code = &code->super;

  x->amino = amino;
  x->nuclt_code = code;
  x->entry_dist = entry_dist;
  x->epsilon = epsilon;
  x->epsilon_frame = imm_frame_epsilon(epsilon);
  memset(x->consensus, 0, array_size_field(struct dcp_protein, consensus));

  dcp_protein_null_init(&x->null, code);
  dcp_protein_alts_init(&x->alts, code);
}

int dcp_protein_set_accession(struct dcp_protein *x, char const *acc)
{
  unsigned n = array_size_field(struct dcp_protein, accession);
  return strkcpy(x->accession, acc, n) ? 0 : DCP_ELONGACC;
}

void dcp_protein_setup(struct dcp_protein *protein, unsigned seq_size,
                       bool multi_hits, bool hmmer3_compat)
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

  dcp_protein_null_setup(&protein->null, &t);
  dcp_protein_alts_setup(&protein->alts, &t);
}

int dcp_protein_absorb(struct dcp_protein *x, struct dcp_model *m)
{
  int rc = 0;

  x->gencode = m->gencode;

  if (x->amino != dcp_model_amino(m)) return DCP_EDIFFABC;
  if (x->nuclt_code->nuclt != dcp_model_nuclt(m)) return DCP_EDIFFABC;

  unsigned n = array_size_field(struct dcp_protein, consensus);
  if (!strkcpy(x->consensus, m->consensus, n)) return DCP_EFORMAT;

  struct dcp_model_summary s = dcp_model_summary(m);
  if ((rc = dcp_protein_null_absorb(&x->null, m, &s))) return rc;
  if ((rc = dcp_protein_alts_absorb(&x->alts, m, &s))) return rc;

  return 0;
}

int dcp_protein_sample(struct dcp_protein *x, unsigned seed, unsigned core_size)
{
  assert(core_size >= 2);
  if (!x->gencode) return DCP_ESETGENCODE;

  x->alts.core_size = core_size;
  struct imm_rnd rnd = imm_rnd(seed);

  float lprobs[IMM_AMINO_SIZE] = {0};

  imm_lprob_sample(&rnd, IMM_AMINO_SIZE, lprobs);
  imm_lprob_normalize(IMM_AMINO_SIZE, lprobs);

  struct dcp_model model = {0};
  dcp_model_init(&model, x->gencode, x->amino, x->nuclt_code, x->entry_dist,
                 x->epsilon, lprobs);

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

  rc = dcp_protein_absorb(x, &model);

cleanup:
  dcp_model_del(&model);
  return rc;
}

int dcp_protein_decode(struct dcp_protein const *x, struct imm_seq const *seq,
                       unsigned state_id, struct imm_codon *codon)
{
  assert(!state_is_mute(state_id));

  struct dcp_nuclt_dist const *nucltd = NULL;
  if (dcp_state_is_insert(state_id))
  {
    nucltd = &x->alts.insert_nuclt_dist;
  }
  else if (dcp_state_is_match(state_id))
  {
    unsigned idx = dcp_state_idx(state_id);
    nucltd = x->alts.match_nuclt_dists + idx;
  }
  else
    nucltd = &x->null.nuclt_dist;

  struct imm_frame_cond cond = {x->epsilon_frame, &nucltd->nucltp,
                                &nucltd->codonm};

  if (imm_lprob_is_nan(imm_frame_cond_decode(&cond, seq, codon)))
    return DCP_EDECODON;

  return 0;
}

void dcp_protein_write_dot(struct dcp_protein const *x, struct imm_dp const *dp,
                           FILE *fp)
{
  imm_dp_write_dot(dp, fp, x->state_name);
}

int dcp_protein_pack(struct dcp_protein const *x, struct lip_file *file)
{
  int rc = 0;
  if (!x->gencode) return DCP_ESETGENCODE;
  if (!lip_write_map_size(file, 5)) return DCP_EFWRITE;

  if (!lip_write_cstr(file, "accession")) return DCP_EFWRITE;
  if (!lip_write_cstr(file, x->accession)) return DCP_EFWRITE;

  if (!lip_write_cstr(file, "gencode")) return DCP_EFWRITE;
  if (!lip_write_int(file, x->gencode->id)) return DCP_EFWRITE;

  if (!lip_write_cstr(file, "consensus")) return DCP_EFWRITE;
  if (!lip_write_cstr(file, x->consensus)) return DCP_EFWRITE;

  if (!lip_write_cstr(file, "null")) return DCP_EFWRITE;
  if ((rc = dcp_protein_null_pack(&x->null, file))) return rc;

  if (!lip_write_cstr(file, "alts")) return DCP_EFWRITE;
  if ((rc = dcp_protein_alts_pack(&x->alts, file))) return rc;
  return 0;
}

int dcp_protein_unpack(struct dcp_protein *x, struct lip_file *file)
{
  unsigned const accession_size =
      array_size_field(struct dcp_protein, accession);
  unsigned const consensus_size =
      array_size_field(struct dcp_protein, consensus);

  int rc = 0;
  if ((rc = dcp_expect_map_size(file, 5))) return rc;

  if ((rc = dcp_expect_map_key(file, "accession"))) return rc;
  if (!lip_read_cstr(file, accession_size, x->accession)) return DCP_EFREAD;

  unsigned gencode_id = 0;
  if ((rc = dcp_expect_map_key(file, "gencode"))) return rc;
  if (!lip_read_int(file, &gencode_id)) return DCP_EFREAD;
  if (!(x->gencode = imm_gencode_get(gencode_id))) return DCP_EFREAD;

  if ((rc = dcp_expect_map_key(file, "consensus"))) return rc;
  if (!lip_read_cstr(file, consensus_size, x->consensus)) return DCP_EFREAD;

  if ((rc = dcp_expect_map_key(file, "null"))) return rc;
  if ((rc = dcp_protein_null_unpack(&x->null, file))) return rc;

  if ((rc = dcp_expect_map_key(file, "alts"))) return rc;
  if ((rc = dcp_protein_alts_unpack(&x->alts, file))) return rc;

  return 0;
}

void dcp_protein_cleanup(struct dcp_protein *x)
{
  if (x)
  {
    x->gencode = NULL;
    dcp_protein_null_cleanup(&x->null);
    dcp_protein_alts_cleanup(&x->alts);
  }
}
