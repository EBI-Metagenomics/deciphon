#include "thread.h"
#include "max.h"
#include "match.h"
#include "chararray.h"
#include "database_reader.h"
#include "debug.h"
#include "error.h"
#include "hmmer_dialer.h"
#include "imm/lprob.h"
#include "lrt.h"
#include "product_line.h"
#include "product_thread.h"
#include "protein_iter.h"
#include "protein_reader.h"
#include "sequence.h"
#include "sequence_queue.h"
#include "trellis.h"
#include "viterbi.h"
#include "window.h"
#include "xsignal.h"
#include <imm/seq.h>
#include <stdlib.h>
#include <string.h>

void thread_init(struct thread *x)
{
  protein_init(&x->protein);

  x->multi_hits = false;
  x->hmmer3_compat = false;

  x->viterbi = NULL;
  x->product = NULL;
  x->partition = -1;
  chararray_init(&x->amino);
  hmmer_init(&x->hmmer);
  x->path = imm_path();
  x->interrupted = false;
}

int thread_setup(struct thread *x, struct thread_params params)
{
  struct database_reader const *db = params.reader->db;
  protein_setup(&x->protein, database_reader_params(db, NULL));
  int rc = 0;

  if ((rc = protein_reader_iter(params.reader, params.partition, &x->iter)))
    return rc;

  x->product = params.product_thread;
  x->partition = params.partition;
  char const *abc = imm_abc_typeid_name(db->nuclt.super.typeid);
  if ((rc = product_line_set_abc(&x->product->line, abc))) return rc;

  x->multi_hits = params.multi_hits;
  x->hmmer3_compat = params.hmmer3_compat;

  if ((rc = hmmer_setup(&x->hmmer, db->has_ga))) return rc;
  if (hmmer_dialer_online(params.dialer))
  {
    if ((rc = hmmer_dialer_dial(params.dialer, &x->hmmer))) return rc;
    if ((rc = hmmer_warmup(&x->hmmer))) return rc;
  }

  x->interrupted = false;

  return (x->viterbi = viterbi_new()) ? 0 : DCP_ENOMEM;
}

void thread_cleanup(struct thread *x)
{
  x->partition = -1;
  protein_cleanup(&x->protein);
  viterbi_del(x->viterbi);
  x->viterbi = NULL;
  chararray_cleanup(&x->amino);
  hmmer_cleanup(&x->hmmer);
  imm_path_cleanup(&x->path);
}

static int process_window(struct thread *, int protein_idx,
                          struct window const *);

int thread_run(struct thread *x, struct sequence_queue const *sequences,
               int *done_proteins, struct xsignal *xsignal,
               bool (*interrupt)(void *), void (*userdata)(void *))
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
      debug("%d:%s:%s", x->partition, x->protein.accession, seq->name);
      struct window w = window_setup(seq, x->protein.core_size);
      int last_hit_pos = -1;
      while (window_next(&w, last_hit_pos))
      {
        int protein_idx = protein_iter_idx(protein_iter);
        if ((rc = process_window(x, protein_idx, &w))) goto cleanup;

        if (interrupt) x->interrupted = (*interrupt)(userdata);
        if (xsignal && xsignal_interrupted(xsignal)) x->interrupted = true;
        if (x->interrupted) goto cleanup;
      }
    }

#pragma omp atomic update
    (*done_proteins)++;
  }

cleanup:
  protein_cleanup(&x->protein);
  return rc;
}

void thread_interrupt(struct thread *x) { x->interrupted = true; }

bool thread_interrupted(struct thread const *x) { return x->interrupted; }

static int code_fn(int pos, int len, void *arg)
{
  struct imm_eseq const *seq = arg;
  return imm_eseq_get(seq, pos, len, 1);
}

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

  int L = sequence_size(seq);
  protein_reset(&x->protein, max(L / 3, 1), multi_hits, hmmer3_compat);
  if ((rc = protein_setup_viterbi(&x->protein, x->viterbi))) return rc;

  float null = -viterbi_null(x->viterbi, sequence_size(seq), code_fn, (void *)&seq->imm.eseq);
  float alt = -viterbi_cost(x->viterbi, sequence_size(seq), code_fn, (void *)&seq->imm.eseq);

  line->lrt = lrt(null, alt);
  if (!imm_lprob_is_finite(line->lrt) || line->lrt < 0) return rc;

  if ((rc = product_line_set_protein(line, x->protein.accession))) return rc;
  if ((rc = viterbi_path(x->viterbi, L, code_fn, (void *)&seq->imm.eseq))) return rc;
  imm_path_reset(&x->path);
  if ((rc = trellis_unzip(viterbi_trellis(x->viterbi), L, &x->path))) return rc;

  struct match begin = match_end();
  struct match end = match_begin(&x->path, &seq->imm.seq, &x->protein);
  line->hit = 0;
  line->hit_start = 0;
  line->hit_stop = 0;

  struct match it = {0};
  while (!match_equal(begin, end))
  {
    line->hit_start = line->hit_stop;
    it = end;
    while (!match_equal(it, match_end()) && match_state_id(&it) != STATE_B)
    {
      line->hit_start += match_step(&it)->seqsize;
      it = match_next(&it);
    }

    line->hit_stop = line->hit_start;
    begin = it;
    while (!match_equal(it, match_end()) && match_state_id(&it) != STATE_E)
    {
      line->hit_stop += match_step(&it)->seqsize;
      it = match_next(&it);
    }
    end = match_next(&it);

    if (match_equal(begin, end)) continue;

    chararray_reset(&x->amino);
    it = begin;
    while (!match_equal(it, end))
    {
      if (!match_state_is_mute(&it))
      {
        char amino = 0;
        if ((rc = match_amino(&it, &amino))) return rc;
        if ((rc = chararray_append(&x->amino, amino))) return rc;
      }
      it = match_next(&it);
    }
    chararray_append(&x->amino, '\0');

    // HMMER3 (2024) cannot handle sequences longer than 100000 letters.
    // https://github.com/EddyRivasLab/hmmer/blob/9acd8b6758a0ca5d21db6d167e0277484341929b/src/p7_pipeline.c#L714
    if (x->amino.size > 100000)
      debug("Amino-acid sequence is too long for HMMER3: %d", x->amino.size);
    if (hmmer_online(&x->hmmer) && x->amino.size <= 100000)
    {
      debug("sending to hmmer sequence of size %zu", x->amino.size);
      if ((rc = hmmer_get(&x->hmmer, protein_idx, seq->name, x->amino.data)))
        return rc;

      x->product->line.evalue = hmmer_result_num_hits(&x->hmmer.result)
                                    ? hmmer_result_evalue(&x->hmmer.result)
                                    : 1.0;
      // We apparently can have num_hits > 0 but without alignment. It seems
      // to happen with evalue is above 1. So lets ignore those hits.
      if (x->product->line.evalue > 1.0) x->product->line.evalue = 1.0;

      if (x->product->line.evalue == 1.0) return rc;
      if ((rc = product_thread_put_hmmer(x->product, &x->hmmer.result)))
        return rc;
    }
    if ((rc = product_thread_put_match(x->product, begin, end))) return rc;
    line->hit += 1;
  }
  return 0;
}
