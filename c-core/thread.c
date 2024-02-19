#include "thread.h"
#include "chararray.h"
#include "database_reader.h"
#include "debug.h"
#include "error.h"
#include "hmmer_dialer.h"
#include "imm/lprob.h"
#include "infer_amino.h"
#include "lrt.h"
#include "match.h"
#include "match_iter.h"
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

static int trim_path(struct protein *, struct imm_seq const *,
                     struct imm_path *, struct imm_seq *, int *seqstart);

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
  if ((rc = protein_setup_viterbi(&x->protein, x->viterbi))) return rc;

  int L = sequence_size(seq);
  float null = -viterbi_null(x->viterbi, sequence_size(seq), code_fn, (void *)&seq->imm.eseq);
  float alt = -viterbi_cost(x->viterbi, sequence_size(seq), code_fn, (void *)&seq->imm.eseq);

  line->lrt = lrt(null, alt);
  if (!imm_lprob_is_finite(line->lrt) || line->lrt < 0) return rc;

  if ((rc = product_line_set_protein(line, x->protein.accession))) return rc;
  if ((rc = viterbi_path(x->viterbi, L, code_fn, (void *)&seq->imm.eseq))) return rc;
  imm_path_reset(&x->path);
  if ((rc = trellis_unzip(viterbi_trellis(x->viterbi), L, &x->path))) return rc;

  struct imm_seq subseq = {0};
  struct imm_path *subpath = &x->path;

  line->hit_start = 0;
  if ((rc = trim_path(&x->protein, &seq->imm.seq, subpath, &subseq,
                      &line->hit_start)))
    return rc;
  line->hit_stop = line->hit_start + imm_seq_size(&subseq);

  if (hmmer_online(&x->hmmer))
  {
    struct match match = match_init(&x->protein);
    struct match_iter mit = {0};
    match_iter_init(&mit, &subseq, subpath);

    if ((rc = infer_amino(&x->amino, &match, &mit))) return rc;

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

  struct match match = match_init(&x->protein);
  struct match_iter mit = {0};
  match_iter_init(&mit, &subseq, subpath);
  return product_thread_put_match(x->product, &match, &mit);
}

static int trim_path(struct protein *protein, struct imm_seq const *seq,
                     struct imm_path *path, struct imm_seq *subseq, int *seqstart)
{
  int rc = 0;

  struct match match = match_init(protein);
  struct match_iter it = {0};
  match_iter_init(&it, seq, path);

  int start = 0;
  int stop = INT_MAX;

  match_iter_rewind(&it);
  while (!(rc = match_iter_next(&it, &match)))
  {
    if (match_iter_end(&it)) return error(DCP_ENOHIT);
    if (match_state_state_id(&match) == STATE_B) break;
  }
  if (rc) return rc;
  start = match_iter_tell(&it) - 1;
  *seqstart = match_iter_seqtell(&it) - match.step.seqsize;

  if ((rc = match_iter_seek(&it, &match, INT_MAX))) return rc;
  while (!(rc = match_iter_prev(&it, &match)))
  {
    if (match_iter_begin(&it)) return error(DCP_ENOHIT);
    if (match_state_state_id(&match) == STATE_E) break;
  }
  if (rc) return rc;
  stop = match_iter_tell(&it) + 1;
  int seqstop = match_iter_seqtell(&it) +  match.step.seqsize;

  match_iter_seek(&it, &match, start);

  *subseq = imm_seq_slice(seq, imm_range(*seqstart, seqstop));
  imm_path_cut(path, start, stop - start);

  return 0;
}
