#include "thread.h"
#include "chararray.h"
#include "database_reader.h"
#include "defer_return.h"
#include "hmmer_dialer.h"
#include "lrt.h"
#include "match.h"
#include "match_iter.h"
#include "now.h"
#include "product_line.h"
#include "product_thread.h"
#include "protein_iter.h"
#include "protein_reader.h"
#include "sequence.h"
#include "sequence_queue.h"
#include "viterbi.h"
#include "viterbi_struct.h"
#include "window.h"

void thread_init(struct thread *x)
{
  protein_init(&x->protein);

  x->multi_hits = false;
  x->hmmer3_compat = false;

  viterbi_init(&x->task);
  x->product = NULL;
  chararray_init(&x->amino);
  hmmer_init(&x->hmmer);
}

int thread_setup(struct thread *x, struct thread_params params)
{
  struct database_reader const *db = params.reader->db;
  protein_setup(&x->protein, database_reader_params(db, NULL));
  int rc = 0;

  if ((rc = protein_reader_iter(params.reader, params.partition, &x->iter)))
    return rc;

  x->product = params.product_thread;
  char const *abc = imm_abc_typeid_name(db->nuclt.super.typeid);
  if ((rc = product_line_set_abc(&x->product->line, abc))) return rc;

  x->multi_hits = params.multi_hits;
  x->hmmer3_compat = params.hmmer3_compat;

  if ((rc = hmmer_setup(&x->hmmer))) return rc;
  if (hmmer_dialer_online(params.dialer))
  {
    if ((rc = hmmer_dialer_dial(params.dialer, &x->hmmer))) return rc;
    if ((rc = hmmer_warmup(&x->hmmer))) return rc;
  }

  return rc;
}

void thread_cleanup(struct thread *x)
{
  protein_cleanup(&x->protein);
  viterbi_cleanup(&x->task);
  chararray_cleanup(&x->amino);
  hmmer_cleanup(&x->hmmer);
}

static int infer_amino(struct chararray *x, struct match *match,
                       struct match_iter *it);

static int run(struct thread *x, int protein_idx, struct window const *w)
{
  int rc = 0;
  struct sequence const *seq = window_sequence(w);
  x->product->line.sequence = seq->id;
  x->product->line.window = w->idx;
  x->product->line.window_start = window_range(w).start;
  x->product->line.window_stop = window_range(w).stop;

  protein_reset(&x->protein, sequence_size(seq), x->multi_hits,
                x->hmmer3_compat);

  if ((rc = viterbi_setup(&x->task, &x->protein, &seq->imm.eseq))) return rc;

  float null = viterbi_null_loglik(&x->task);
  float alt = viterbi_alt_loglik(&x->task);

  if (!imm_lprob_is_finite(lrt(null, alt)) || alt < null) return rc;
  x->product->line.lrt = lrt(null, alt);

  if ((rc = viterbi_alt_path(&x->task, NULL))) return rc;

  if ((rc = product_line_set_protein(&x->product->line, x->protein.accession)))
    return rc;

  struct match match = match_init(&x->protein);
  struct match_iter mit = {0};

  match_iter_init(&mit, &seq->imm.seq, &x->task.path);
  if ((rc = infer_amino(&x->amino, &match, &mit))) return rc;

  if (hmmer_online(&x->hmmer))
  {
    if ((rc = hmmer_get(&x->hmmer, protein_idx, seq->name, x->amino.data)))
      return rc;
    if (hmmer_result_num_hits(&x->hmmer.result) == 0) return rc;
    x->product->line.evalue = hmmer_result_evalue(&x->hmmer.result);
    if ((rc = product_thread_put_hmmer(x->product, &x->hmmer.result)))
      return rc;
  }

  match_iter_init(&mit, &seq->imm.seq, &x->task.path);
  if ((rc = product_thread_put_match(x->product, &match, &mit))) return rc;
  return rc;
}

int thread_run(struct thread *x, struct sequence_queue const *sequences)
{
  int rc = 0;

  struct protein_iter *protein_iter = &x->iter;

  if ((rc = protein_iter_rewind(protein_iter))) goto cleanup;

  while (!(rc = protein_iter_next(protein_iter, &x->protein)))
  {
    if (protein_iter_end(protein_iter)) break;

    struct sequence *seq = NULL;
    struct iter seq_iter = sequence_queue_iter(sequences);
    iter_for_each_entry(seq, &seq_iter, node)
    {
      struct window w = window_setup(seq, x->protein.core_size);
      int last_hit_pos = -1;
      while (window_next(&w, last_hit_pos))
      {
        int protein_idx = protein_iter_idx(protein_iter);
        if ((rc = run(x, protein_idx, &w))) break;
      }
    }
  }

cleanup:
  protein_cleanup(&x->protein);
  return rc;
}

static int infer_amino(struct chararray *x, struct match *match,
                       struct match_iter *it)
{
  int rc = 0;

  chararray_reset(x);
  while (!(rc = match_iter_next(it, match)))
  {
    if (match_iter_end(it)) break;
    if (!match_state_is_core(match)) continue;
    if (match_state_is_mute(match)) continue;
    if ((rc = chararray_append(x, match_amino(match)))) return rc;
  }

  return chararray_append(x, '\0');
}
