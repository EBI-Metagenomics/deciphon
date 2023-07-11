#include "scan_thrd.h"
#include "chararray.h"
#include "db_reader.h"
#include "defer_return.h"
#include "hmmer_dialer.h"
#include "lrt.h"
#include "match.h"
#include "match_iter.h"
#include "prod_match.h"
#include "prod_writer_thrd.h"
#include "protein.h"
#include "protein_iter.h"
#include "protein_reader.h"
#include "seq.h"
#include "seq_struct.h"

int dcp_scan_thrd_init(struct dcp_scan_thrd *x,
                       struct dcp_protein_reader *reader, int partition,
                       struct dcp_prod_writer_thrd *prod_thrd,
                       struct dcp_hmmer_dialer *dialer)
{
  struct dcp_db_reader const *db = reader->db;
  dcp_protein_init(&x->protein, NULL, &db->amino, &db->code, db->entry_dist,
                   db->epsilon);
  dcp_protein_reader_iter(reader, partition, &x->iter);

  x->prod_thrd = prod_thrd;
  struct imm_abc const *abc = &db->nuclt.super;
  char const *abc_name = imm_abc_typeid_name(abc->typeid);
  dcp_prod_match_set_abc(&x->prod_thrd->match, abc_name);

  dcp_chararray_init(&x->amino);

  int rc = 0;
  if ((rc = dcp_hmmer_init(&x->hmmer))) defer_return(rc);
  if ((rc = dcp_hmmer_dialer_dial(dialer, &x->hmmer))) defer_return(rc);
  if ((rc = dcp_hmmer_warmup(&x->hmmer))) defer_return(rc);

  return rc;

defer:
  dcp_protein_cleanup(&x->protein);
  dcp_chararray_cleanup(&x->amino);
  return rc;
}

void dcp_scan_thrd_cleanup(struct dcp_scan_thrd *x)
{
  dcp_protein_cleanup(&x->protein);
  dcp_chararray_cleanup(&x->amino);
  dcp_hmmer_cleanup(&x->hmmer);
}

void dcp_scan_thrd_set_lrt_threshold(struct dcp_scan_thrd *x, double lrt)
{
  x->lrt_threshold = lrt;
}

void dcp_scan_thrd_set_multi_hits(struct dcp_scan_thrd *x, bool multihits)
{
  x->multi_hits = multihits;
}

void dcp_scan_thrd_set_hmmer3_compat(struct dcp_scan_thrd *x, bool h3compat)
{
  x->hmmer3_compat = h3compat;
}

static int infer_amino(struct dcp_chararray *x, struct dcp_match *match,
                       struct dcp_match_iter *it);

int dcp_scan_thrd_run(struct dcp_scan_thrd *x, struct dcp_seq const *seq)
{
  int rc = 0;
  struct dcp_scan_task null = {0};
  struct dcp_scan_task alt = {0};

  struct dcp_protein_iter *it = &x->iter;
  x->prod_thrd->match.seq_id = dcp_seq_id(seq);

  if ((rc = dcp_protein_iter_rewind(it))) goto cleanup;

  while (!(rc = dcp_protein_iter_next(it, &x->protein)))
  {
    if (dcp_protein_iter_end(it)) break;

    struct imm_dp const *null_dp = &x->protein.null.dp;
    struct imm_dp const *alt_dp = &x->protein.alts.full.dp;

    rc = dcp_scan_task_setup(&null, &x->protein.null.dp, seq);
    if (rc) goto cleanup;

    rc = dcp_scan_task_setup(&alt, &x->protein.alts.full.dp, seq);
    if (rc) goto cleanup;

    dcp_protein_setup(&x->protein, dcp_seq_size(seq), x->multi_hits,
                      x->hmmer3_compat);

    if (imm_dp_viterbi(null_dp, null.task, &null.prod)) goto cleanup;
    if (imm_dp_viterbi(alt_dp, alt.task, &alt.prod)) goto cleanup;

    x->prod_thrd->match.null = null.prod.loglik;
    x->prod_thrd->match.alt = alt.prod.loglik;

    float lrt = dcp_prod_match_get_lrt(&x->prod_thrd->match);

    if (!imm_lprob_is_finite(lrt) || lrt < x->lrt_threshold) continue;

    dcp_prod_match_set_protein(&x->prod_thrd->match, x->protein.accession);

    struct dcp_match match = {0};
    dcp_match_init(&match, &x->protein);

    struct dcp_match_iter mit = {0};

    dcp_match_iter_init(&mit, dcp_seq_imm_seq(seq), &alt.prod.path);
    if ((rc = infer_amino(&x->amino, &match, &mit))) break;
    if ((rc = dcp_hmmer_get(&x->hmmer, dcp_protein_iter_idx(it), seq->name,
                            x->amino.data)))
      break;
    if (dcp_hmmer_result_nhits(&x->hmmer.result) == 0) continue;
    x->prod_thrd->match.evalue = dcp_hmmer_result_evalue_ln(&x->hmmer.result);
    if ((rc = dcp_prod_writer_thrd_put_hmmer(x->prod_thrd, &x->hmmer.result)))
      break;

    dcp_match_iter_init(&mit, dcp_seq_imm_seq(seq), &alt.prod.path);
    if ((rc = dcp_prod_writer_thrd_put(x->prod_thrd, &match, &mit))) break;
  }

cleanup:
  dcp_protein_cleanup(&x->protein);
  dcp_scan_task_cleanup(&null);
  dcp_scan_task_cleanup(&alt);
  return rc;
}

static int infer_amino(struct dcp_chararray *x, struct dcp_match *match,
                       struct dcp_match_iter *it)
{
  int rc = 0;

  dcp_chararray_reset(x);
  while (!(rc = dcp_match_iter_next(it, match)))
  {
    if (dcp_match_iter_end(it)) break;
    if (dcp_match_state_is_mute(match)) continue;
    if ((rc = dcp_chararray_append(x, dcp_match_amino(match)))) return rc;
  }

  return dcp_chararray_append(x, '\0');
}
