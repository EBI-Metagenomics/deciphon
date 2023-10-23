#include "codec.h"
#include "imm/imm.h"
#include "protein.h"
#include "state.h"

struct codec codec_init(struct protein const *protein,
                        struct imm_path const *path)
{
  return (struct codec){0, 0, protein, path};
}

int codec_next(struct codec *x, struct imm_seq const *seq,
               struct imm_codon *codon)
{
  struct imm_step const *step = NULL;

  for (; x->idx < imm_path_nsteps(x->path); x->idx++)
  {
    step = imm_path_step(x->path, x->idx);
    if (!state_is_mute(step->state_id)) break;
  }

  if (codec_end(x)) return 0;

  int size = step->seqsize;
  struct imm_range range = imm_range(x->start, x->start + size);
  struct imm_seq frag = imm_seq_slice(seq, range);
  x->start += size;
  x->idx++;
  return protein_decode(x->protein, &frag, step->state_id, codon);
}

bool codec_end(struct codec const *x)
{
  return x->idx >= imm_path_nsteps(x->path);
}
