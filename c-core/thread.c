#include "thread.h"
#include "batch.h"
#include "chararray.h"
#include "debug.h"
#include "h3r_result.h"
#include "hmmer.h"
#include "imm_lprob.h"
#include "iter.h"
#include "lrt.h"
#include "match.h"
#include "max.h"
#include "product_line.h"
#include "product_thread.h"
#include "sequence.h"
#include "trellis.h"
#include "viterbi.h"
#include "window.h"
#include "workload.h"
#include <stdlib.h>
#include <string.h>

void thread_init(struct thread *x)
{
  x->workload = NULL;
  chararray_init(&x->amino);
  x->hmmer = NULL;
  x->path  = imm_path();
  x->interrupted = false;
}

void thread_setup(struct thread *x, struct hmmer *hmmer, struct workload *workload)
{
  x->workload    = workload;
  x->hmmer       = hmmer;
  x->interrupted = false;
}

void thread_cleanup(struct thread *x)
{
  x->workload = NULL;
  chararray_cleanup(&x->amino);
  imm_path_cleanup(&x->path);
}

static int process_window(struct thread *, struct work *work, int protein_idx,
                          struct product_thread *, struct window *);

int thread_run(struct thread *x, struct dcp_batch const *batch,
               int *done_proteins, void (*callb)(void *),
               void (*userdata)(void *), struct product_thread *product)
{
  int rc = 0;
  x->interrupted = false;
  struct work *work = NULL;

  if ((rc = workload_rewind(x->workload))) return rc;

  while (!(rc = workload_next(x->workload, &work)))
  {
    if (workload_end(x->workload)) break;

    struct sequence *seq = NULL;
    struct iter seq_iter = batch_iter(batch);
    iter_for_each_entry(seq, &seq_iter, node)
    {
      debug("%s:%s", work->accession, seq->name);
      struct window w = window_setup(seq, work->core_size);
      while (window_next(&w))
      {
        int idx = workload_index(x->workload);
        if ((rc = process_window(x, work, idx, product, &w))) return rc;

        if (callb) (*callb)(userdata);
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

static int process_window(struct thread *x, struct work *work, int protein_idx,
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
  work_reset(work, max(L / 3, 1));

  float null = -viterbi_null(work->viterbi, sequence_size(seq), code_fn,
                             (void *)&seq->imm.eseq);
  float alt = -viterbi_cost(work->viterbi, sequence_size(seq), code_fn,
                            (void *)&seq->imm.eseq);

  line->lrt = lrt(null, alt);
  debug("lrt for %s: %g", work->accession, line->lrt);
  if (!imm_lprob_is_finite(line->lrt) || line->lrt < 0) return rc;
  debug("passed lrt threshold for window [%d,%d]", window_range(w).start,
        window_range(w).stop);

  if ((rc = product_line_set_accession(line, work->accession))) return rc;
  if ((rc = viterbi_path(work->viterbi, L, code_fn, (void *)&seq->imm.eseq)))
    return rc;
  imm_path_reset(&x->path);
  if ((rc = trellis_unzip(viterbi_trellis(work->viterbi), L, &x->path))) return rc;

  struct match begin = match_end();
  struct match end = match_begin(&x->path, &seq->imm.seq, &work->decoder);
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
