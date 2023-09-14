#include "scan_thrd.h"
#include "chararray.h"
#include "db_reader.h"
#include "defer_return.h"
// #include "elapsed/elapsed.h"
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
#include "vit.h"

void dcp_scan_thrd_init(struct dcp_scan_thrd *x)
{
  *x = (struct dcp_scan_thrd){};
}

int dcp_scan_thrd_setup(struct dcp_scan_thrd *x,
                        struct dcp_scan_thrd_params params)
{
  struct dcp_db_reader const *db = params.reader->db;
  dcp_protein_init(&x->protein, dcp_db_reader_params(db, NULL));
  p7_init(&x->p7, dcp_db_reader_params(db, NULL));
  int rc = 0;

  if ((rc = dcp_protein_reader_iter(params.reader, params.partition, &x->iter)))
    defer_return(rc);

  x->prod_thrd = params.prod_thrd;
  char const *abc_name = imm_abc_typeid_name(db->nuclt.super.typeid);
  dcp_prod_match_set_abc(&x->prod_thrd->match, abc_name);

  dcp_chararray_init(&x->amino);
  x->lrt_threshold = params.lrt_threshold;
  x->multi_hits = params.multi_hits;
  x->hmmer3_compat = params.hmmer3_compat;
  x->disable_hmmer = params.disable_hmmer;

  if ((rc = dcp_hmmer_init(&x->hmmer))) defer_return(rc);
  if (!x->disable_hmmer)
  {
    if ((rc = dcp_hmmer_dialer_dial(params.dialer, &x->hmmer)))
      defer_return(rc);
    if ((rc = dcp_hmmer_warmup(&x->hmmer))) defer_return(rc);
  }

  return rc;

defer:
  p7_cleanup(&x->p7);
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

  // long new_mseconds_null = 0;
  // long new_mseconds_alt = 0;
  // long old_mseconds_null = 0;
  // long old_mseconds_alt = 0;
  // struct elapsed elapsed = ELAPSED_INIT;
  // int num_iters = 0;
  while (!(rc = dcp_protein_iter_next(it, &x->protein, &x->p7)))
  {
    if (dcp_protein_iter_end(it)) break;
    // num_iters++;

    struct imm_dp const *null_dp = &x->protein.null.dp;
    struct imm_dp const *alt_dp = &x->protein.alts.full.dp;

    rc = dcp_scan_task_setup(&null, &x->protein.null.dp, seq);
    if (rc) goto cleanup;

    rc = dcp_scan_task_setup(&alt, &x->protein.alts.full.dp, seq);
    if (rc) goto cleanup;

    dcp_protein_setup(&x->protein, dcp_seq_size(seq), x->multi_hits,
                      x->hmmer3_compat);
    p7_setup(&x->p7, dcp_seq_size(seq), x->multi_hits, x->hmmer3_compat);

    // if (elapsed_start(&elapsed)) return IMM_EELAPSED;
    // null.prod.loglik = dcp_vit_null(&x->p7, &seq->imm_eseq);
    // if (elapsed_stop(&elapsed)) return IMM_EELAPSED;
    // new_mseconds_null += elapsed_milliseconds(&elapsed);

    // if (elapsed_start(&elapsed)) return IMM_EELAPSED;
    // alt.prod.loglik = dcp_vit(&x->p7, &seq->imm_eseq);
    // if (elapsed_stop(&elapsed)) return IMM_EELAPSED;
    // new_mseconds_alt += elapsed_milliseconds(&elapsed);

    float fast_null = dcp_vit_null(&x->p7, &seq->imm_eseq);
    float fast_alt = dcp_vit(&x->p7, &seq->imm_eseq);
    if (imm_dp_viterbi(null_dp, null.task, &null.prod)) goto cleanup;
    if (imm_dp_viterbi(alt_dp, alt.task, &alt.prod)) goto cleanup;
    float slow_null = null.prod.loglik;
    float slow_alt = alt.prod.loglik;

    // x->prod_thrd->match.null = slow_null;
    // x->prod_thrd->match.alt = slow_alt;
    x->prod_thrd->match.null = fast_null;
    x->prod_thrd->match.alt = fast_alt;

    // printf("%g %g\n", fast_null - slow_null, fast_alt - slow_alt);
    // printf("%d: %g\n", dcp_seq_size(seq), fast_alt - slow_alt);
    // printf("SLOW: %g\n", slow_alt);
    // printf("FAST: %g\n", fast_alt);
    imm_dp_set_state_name((struct imm_dp *)alt_dp, x->protein.state_name);
    imm_trellis_set_state_table(&alt.task->trellis, &alt_dp->state_table);
    // imm_trellis_dump(&alt.task->trellis, stdout);
    // imm_dp_dump(alt_dp, stdout);

    // x->prod_thrd->match.null = null.prod.loglik;
    // x->prod_thrd->match.alt = alt.prod.loglik;
    // float lrt = dcp_prod_match_get_lrt(&x->prod_thrd->match);
    // printf("%.9g\n", null.prod.loglik);
    // printf("%g\n", lrt);
    // if (!imm_lprob_is_finite(lrt) || lrt < x->lrt_threshold) continue;

    // if (elapsed_start(&elapsed)) return IMM_EELAPSED;
    // if (imm_dp_viterbi(null_dp, null.task, &null.prod)) goto cleanup;
    // if (elapsed_stop(&elapsed)) return IMM_EELAPSED;
    // old_mseconds_null += elapsed_milliseconds(&elapsed);

    // if (elapsed_start(&elapsed)) return IMM_EELAPSED;
    // if (imm_dp_viterbi(alt_dp, alt.task, &alt.prod)) goto cleanup;
    // if (elapsed_stop(&elapsed)) return IMM_EELAPSED;
    // old_mseconds_alt += elapsed_milliseconds(&elapsed);
    //
    // x->prod_thrd->match.null = null.prod.loglik;
    // x->prod_thrd->match.alt = alt.prod.loglik;
    float lrt = dcp_prod_match_get_lrt(&x->prod_thrd->match);
    // printf("%g\n", lrt);
    if (!imm_lprob_is_finite(lrt) || lrt < x->lrt_threshold) continue;

    // assert(fabs(null.prod.loglik - x->prod_thrd->match.null) < 1e-7);
    // assert(fabs(alt.prod.loglik - x->prod_thrd->match.alt) < 1e-7);
    // if (fabs(null.prod.loglik - x->prod_thrd->match.null) > 1e-7) exit(1);
    // if (fabs(alt.prod.loglik - x->prod_thrd->match.alt) > 1e-7) exit(1);

    dcp_prod_match_set_protein(&x->prod_thrd->match, x->protein.accession);

    struct dcp_match match = {0};
    dcp_match_init(&match, &x->protein);

    struct dcp_match_iter mit = {0};

    dcp_match_iter_init(&mit, dcp_seq_immseq(seq), &alt.prod.path);
    if ((rc = infer_amino(&x->amino, &match, &mit))) break;

    if (!x->disable_hmmer)
    {
      if ((rc = dcp_hmmer_get(&x->hmmer, dcp_protein_iter_idx(it), seq->name,
                              x->amino.data)))
        break;
      if (dcp_hmmer_result_nhits(&x->hmmer.result) == 0) continue;
      x->prod_thrd->match.evalue = dcp_hmmer_result_evalue_ln(&x->hmmer.result);
      if ((rc = dcp_prod_writer_thrd_put_hmmer(x->prod_thrd, &x->hmmer.result)))
        break;
    }

    dcp_match_iter_init(&mit, dcp_seq_immseq(seq), &alt.prod.path);
    if ((rc = dcp_prod_writer_thrd_put(x->prod_thrd, &match, &mit))) break;
  }

  // printf("Total number of iters: %d\n", num_iters);
  // printf("new_nseconds_null: %ld\n",
  //        (long)(1000 * new_mseconds_null / (float)num_iters));
  // printf("new_nseconds_alt: %ld\n",
  //        (long)(1000 * new_mseconds_alt / (float)num_iters));
  // printf("old_nseconds_null: %ld\n",
  //        (long)(1000 * old_mseconds_null / (float)num_iters));
  // printf("old_nseconds_alt: %ld\n",
  //        (long)(1000 * old_mseconds_alt / (float)num_iters));

cleanup:
  p7_cleanup(&x->p7);
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
