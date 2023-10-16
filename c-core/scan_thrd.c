#include "scan_thrd.h"
#include "chararray.h"
#include "db_reader.h"
#include "defer_return.h"
#include "hmmer_dialer.h"
#include "lrt.h"
#include "match.h"
#include "match_iter.h"
#include "now.h"
#include "prod_match.h"
#include "prod_writer_thrd.h"
#include "protein_iter.h"
#include "protein_reader.h"
#include "seq.h"
#include "seq_struct.h"
#include "viterbi.h"
#include "window.h"

void dcp_scan_thrd_init(struct dcp_scan_thrd *x)
{
  *x = (struct dcp_scan_thrd){};
  dcp_viterbi_task_init(&x->task);
}

int dcp_scan_thrd_setup(struct dcp_scan_thrd *x,
                        struct dcp_scan_thrd_params params)
{
  struct dcp_db_reader const *db = params.reader->db;
  protein_init(&x->protein, dcp_db_reader_params(db, NULL));
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
  protein_cleanup(&x->protein);
  dcp_chararray_cleanup(&x->amino);
  return rc;
}

void dcp_scan_thrd_cleanup(struct dcp_scan_thrd *x)
{
  dcp_viterbi_task_cleanup(&x->task);
  dcp_chararray_cleanup(&x->amino);
  dcp_hmmer_cleanup(&x->hmmer);
}

static int infer_amino(struct dcp_chararray *x, struct dcp_match *match,
                       struct dcp_match_iter *it);

static int run(struct dcp_scan_thrd *x, int protein_idx, struct window const *w)
{
  int rc = 0;
  struct dcp_seq const *seq = window_sequence(w);
  x->prod_thrd->match.seq_id = dcp_seq_id(seq);

  protein_setup(&x->protein, dcp_seq_size(seq), x->multi_hits,
                x->hmmer3_compat);

  x->prod_thrd->match.null = dcp_viterbi_null(&x->protein, &seq->imm_eseq);

  if ((rc = dcp_viterbi(&x->protein, &seq->imm_eseq, &x->task, true)))
    return rc;
  x->prod_thrd->match.alt = x->task.score;

  float lrt = dcp_prod_match_get_lrt(&x->prod_thrd->match);
  if (!imm_lprob_is_finite(lrt) || lrt < x->lrt_threshold) return rc;

  if ((rc = dcp_viterbi(&x->protein, &seq->imm_eseq, &x->task, false)))
    return rc;
  assert(fabs(x->prod_thrd->match.alt - x->task.score) < 1e-7);

  dcp_prod_match_set_protein(&x->prod_thrd->match, x->protein.accession);

  struct dcp_match match = {0};
  dcp_match_init(&match, &x->protein);

  struct dcp_match_iter mit = {0};

  dcp_match_iter_init(&mit, dcp_seq_immseq(seq), &x->task.path);
  if ((rc = infer_amino(&x->amino, &match, &mit))) return rc;

  if (!x->disable_hmmer)
  {
    if ((rc = dcp_hmmer_get(&x->hmmer, protein_idx, seq->name, x->amino.data)))
      return rc;
    if (dcp_hmmer_result_nhits(&x->hmmer.result) == 0) return rc;
    x->prod_thrd->match.evalue = dcp_hmmer_result_evalue_ln(&x->hmmer.result);
    if ((rc = dcp_prod_writer_thrd_put_hmmer(x->prod_thrd, &x->hmmer.result)))
      return rc;
  }

  dcp_match_iter_init(&mit, dcp_seq_immseq(seq), &x->task.path);
  if ((rc = dcp_prod_writer_thrd_put(x->prod_thrd, &match, &mit))) return rc;
  return rc;
}

int dcp_scan_thrd_run(struct dcp_scan_thrd *x, struct queue const *seqs)
{
  int rc = 0;

  struct dcp_protein_iter *protein_iter = &x->iter;

  if ((rc = dcp_protein_iter_rewind(protein_iter))) goto cleanup;

  while (!(rc = dcp_protein_iter_next(protein_iter, &x->protein)))
  {
    if (dcp_protein_iter_end(protein_iter)) break;

    struct dcp_seq *seq = NULL;
    struct iter seq_iter = queue_iter(seqs);
    iter_for_each_entry(seq, &seq_iter, node)
    {
      struct window w = window_setup(seq, x->protein.core_size);
      int last_hit_pos = -1;
      while (window_next(&w, last_hit_pos))
      {
        int protein_idx = dcp_protein_iter_idx(protein_iter);
        if ((rc = run(x, protein_idx, &w))) break;
      }
    }
  }

cleanup:
  protein_cleanup(&x->protein);
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
    if (!dcp_match_state_is_core(match)) continue;
    if (dcp_match_state_is_mute(match)) continue;
    if ((rc = dcp_chararray_append(x, dcp_match_amino(match)))) return rc;
  }

  return dcp_chararray_append(x, '\0');
}
