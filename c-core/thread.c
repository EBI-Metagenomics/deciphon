#include "thread.h"
#include "batch.h"
#include "chararray.h"
#include "debug.h"
#include "error.h"
#include "h3r_result.h"
#include "hmmer.h"
#include "imm_lprob.h"
#include "iter.h"
#include "lrt.h"
#include "match.h"
#include "max.h"
#include "product_line.h"
#include "product_thread.h"
#include "protein_iter.h"
#include "protein_reader.h"
#include "sequence.h"
#include "trellis.h"
#include "viterbi.h"
#include "window.h"
#include "xsignal.h"
#include <stdlib.h>
#include <string.h>

void thread_init(struct thread *x)
{
  x->protein = NULL;
  x->iter = NULL;
  x->viterbi = NULL;
  chararray_init(&x->amino);
  x->hmmer = NULL;
  x->path = imm_path();
  x->interrupted = false;
}

int thread_setup(struct thread *x, struct hmmer *hmmer, struct protein *protein,
                 struct protein_iter *iter)
{
  x->protein = protein;
  x->iter = iter;
  x->hmmer = hmmer;
  x->interrupted = false;
  return (x->viterbi = viterbi_new()) ? 0 : DCP_ENOMEM;
}

void thread_cleanup(struct thread *x)
{
  viterbi_del(x->viterbi);
  x->viterbi = NULL;
  chararray_cleanup(&x->amino);
  imm_path_cleanup(&x->path);
}

static int process_window(struct thread *, int protein_idx,
                          struct product_thread *, struct window *);

int thread_run(struct thread *x, struct batch const *batch,
               int *done_proteins, struct xsignal *xsignal,
               bool (*interrupt)(void *), void (*userdata)(void *),
               struct product_thread *product)
{
  int rc = 0;
  x->interrupted = false;

  struct protein_iter *protein_iter = x->iter;

  if ((rc = protein_iter_rewind(protein_iter))) return rc;

  while (!(rc = protein_iter_next(protein_iter, x->protein)))
  {
    if (protein_iter_end(protein_iter)) break;

    struct sequence *seq = NULL;
    struct iter seq_iter = batch_iter(batch);
    iter_for_each_entry(seq, &seq_iter, node)
    {
      debug("%s:%s", x->protein->accession, seq->name);
      struct window w = window_setup(seq, x->protein->core_size);
      while (window_next(&w))
      {
        int protein_idx = protein_iter_idx(protein_iter);
        if ((rc = process_window(x, protein_idx, product, &w))) return rc;

        if (interrupt) x->interrupted = (*interrupt)(userdata);
        if (xsignal && xsignal_interrupted(xsignal)) x->interrupted = true;
        if (x->interrupted) return rc;
      }
    }

#pragma omp atomic update
    (*done_proteins)++;
  }

  chararray_reset(&x->amino);
  imm_path_reset(&x->path);
  return 0;
}

void thread_interrupt(struct thread *x) { x->interrupted = true; }

bool thread_interrupted(struct thread const *x) { return x->interrupted; }

static int code_fn(int pos, int len, void *arg)
{
  struct imm_eseq const *seq = arg;
  return imm_eseq_get(seq, pos, len, 1);
}

static int process_window(struct thread *x, int protein_idx,
                          struct product_thread *product, struct window *w)
{
  int rc = 0;
  struct sequence const *seq = window_sequence(w);
  struct product_line *line = &product->line;
  line->sequence = seq->id;
  line->window = w->idx;
  line->window_start = window_range(w).start;
  line->window_stop = window_range(w).stop;
  debug("running on window [%d,%d]", window_range(w).start,
        window_range(w).stop);

  int L = sequence_size(seq);
  protein_reset(x->protein, max(L / 3, 1));
  if ((rc = protein_setup_viterbi(x->protein, x->viterbi))) return rc;

  float null = -viterbi_null(x->viterbi, sequence_size(seq), code_fn,
                             (void *)&seq->imm.eseq);
  float alt = -viterbi_cost(x->viterbi, sequence_size(seq), code_fn,
                            (void *)&seq->imm.eseq);

  line->lrt = lrt(null, alt);
  debug("lrt for %s: %g", x->protein->accession, line->lrt);
  if (!imm_lprob_is_finite(line->lrt) || line->lrt < 0) return rc;
  debug("passed lrt threshold for window [%d,%d]", window_range(w).start,
        window_range(w).stop);

  if ((rc = product_line_set_accession(line, x->protein->accession))) return rc;
  if ((rc = viterbi_path(x->viterbi, L, code_fn, (void *)&seq->imm.eseq)))
    return rc;
  imm_path_reset(&x->path);
  if ((rc = trellis_unzip(viterbi_trellis(x->viterbi), L, &x->path))) return rc;

  struct match begin = match_end();
  struct match end = match_begin(&x->path, &seq->imm.seq, x->protein);
  line->hit = 0;
  line->hit_start = 0;
  line->hit_stop = 0;

  if (match_equal(begin, end)) return rc;

  struct match it = {0};
  line->hit_start = line->hit_stop;
  it = end;
  while (!match_equal(it, match_end()) && match_state_id(&it) != STATE_B)
  {
    line->hit_start += match_step(&it)->seqsize;
    it = match_next(&it);
  }
  if (match_equal(it, match_end())) return rc;
  int hit_stop = line->hit_start;
  begin = it;
  end = match_next(&it);

  for (;;)
  {
    it = end;
    line->hit_stop = hit_stop;
    while (!match_equal(it, match_end()) && match_state_id(&it) != STATE_E)
    {
      hit_stop += match_step(&it)->seqsize;
      it = match_next(&it);
    }
    if (match_equal(it, match_end()))
    {
      window_set_last_hit_position(w, line->hit_stop - 1);
      break;
    }
    end = match_next(&it);
  }

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
  if (x->amino.size <= 100000)
  {
    debug("sending to hmmer sequence of size %zu", x->amino.size);
    debug("sequence sent: %s", x->amino.data);
    if ((rc = hmmer_get(x->hmmer, protein_idx, x->amino.data))) return rc;

    product->line.logevalue = h3r_nhits(x->hmmer->result)
                                  ? h3r_hit_logevalue(x->hmmer->result, 0)
                                  : 1.0;
    // We apparently can have num_hits > 0 but without alignment. It seems
    // to happen with evalue is above 1. So lets ignore those hits.
    if (product->line.logevalue > 0) product->line.logevalue = 0;

    debug("num_hits: %d logvalue: %g", h3r_nhits(x->hmmer->result),
          product->line.logevalue);
    if (product->line.logevalue == 0) return rc;
    if ((rc = product_thread_add_hmmer(product, x->hmmer->result))) return rc;
  }
  if ((rc = product_thread_add_match(product, begin, end))) return rc;
  line->hit += 1;

  return 0;
}
