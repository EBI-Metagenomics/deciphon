#include "thread.h"
#include "chararray.h"
#include "database_reader.h"
#include "defer_return.h"
#include "hmmer_dialer.h"
#include "infer_amino.h"
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

  viterbi_init(&x->viterbi);
  x->product = NULL;
  chararray_init(&x->amino);
  hmmer_init(&x->hmmer);
  x->path = imm_path();
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
  viterbi_cleanup(&x->viterbi);
  chararray_cleanup(&x->amino);
  hmmer_cleanup(&x->hmmer);
  imm_path_cleanup(&x->path);
}

static int process_window(struct thread *, int protein_idx,
                          struct window const *);

int thread_run(struct thread *x, struct sequence_queue const *sequences,
               int *done_proteins)
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
        if ((rc = process_window(x, protein_idx, &w))) break;
      }
    }

#pragma omp atomic update
    (*done_proteins)++;
  }

cleanup:
  protein_cleanup(&x->protein);
  return rc;
}

static int hmmer_stage(struct protein *, struct product_thread *,
                       struct hmmer *, struct chararray *,
                       struct sequence const *, int protein_idx,
                       struct imm_path const *);

static int process_window(struct thread *x, int protein_idx,
                          struct window const *w)
{
  int rc = 0;
  struct sequence const *seq = window_sequence(w);
  struct product_line *line = &x->product->line;
  line->sequence = seq->id;
  line->window = w->idx;
  line->window_start = window_range(w).start;
  line->window_stop = window_range(w).stop;
  bool multi_hits = x->multi_hits;
  bool hmmer3_compat = x->hmmer3_compat;

  protein_reset(&x->protein, sequence_size(seq), multi_hits, hmmer3_compat);
  if ((rc = viterbi_setup(&x->viterbi, &x->protein, &seq->imm.eseq))) return rc;

  float null = viterbi_null_loglik(&x->viterbi);
  float alt = viterbi_alt_loglik(&x->viterbi);
  line->lrt = lrt(null, alt);
  if (!imm_lprob_is_finite(line->lrt) || line->lrt < 0) return rc;

  if ((rc = product_line_set_protein(line, x->protein.accession))) return rc;
  if ((rc = viterbi_alt_path(&x->viterbi, &x->path, NULL))) return rc;

  if (hmmer_online(&x->hmmer))
  {
    if ((rc = hmmer_stage(&x->protein, x->product, &x->hmmer, &x->amino, seq,
                          protein_idx, &x->path)))
      return rc;
  }

  struct match match = match_init(&x->protein);
  struct match_iter mit = {0};
  match_iter_init(&mit, &seq->imm.seq, &x->path);
  return product_thread_put_match(x->product, &match, &mit);
}

static int hmmer_stage(struct protein *protein, struct product_thread *product,
                       struct hmmer *hmmer, struct chararray *amino,
                       struct sequence const *seq, int protein_idx,
                       struct imm_path const *path)
{
  int rc = 0;
  struct match match = match_init(protein);
  struct match_iter mit = {0};
  match_iter_init(&mit, &seq->imm.seq, path);

  if ((rc = infer_amino(amino, &match, &mit))) return rc;

  if ((rc = hmmer_get(hmmer, protein_idx, seq->name, amino->data))) return rc;
  if (hmmer_result_num_hits(&hmmer->result) == 0) return rc;

  product->line.evalue = hmmer_result_evalue(&hmmer->result);
  return product_thread_put_hmmer(product, &hmmer->result);
}
