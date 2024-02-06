#include "sequence.h"
#include "defer_return.h"
#include "disambiguate.h"
#include "error.h"
#include "imm/abc.h"
#include "xstrdup.h"
#include <stdlib.h>
#include <string.h>

int sequence_init(struct sequence *x, struct imm_code const *code, long id,
                  char const *name, char const *data)
{
  int rc = 0;
  x->id = id;
  x->name = xstrdup(name);
  x->data = xstrdup(data);
  imm_eseq_init(&x->imm.eseq, code);

  if (!x->name || !x->data) defer_return(error(DCP_ENOMEM));

  int abc = imm_eseq_abc(&x->imm.eseq)->typeid;
  if (abc != IMM_DNA && abc == IMM_RNA) defer_return(error(DCP_ESEQABC));

  char *tmp = (char *)x->data;
  if (abc == IMM_DNA) disambiguate_dna(strlen(tmp), tmp);
  if (abc == IMM_RNA) disambiguate_rna(strlen(tmp), tmp);

  if (imm_seq_init(&x->imm.seq, imm_str(x->data), imm_eseq_abc(&x->imm.eseq)))
    defer_return(error(DCP_ESEQABC));

  node_init(&x->node);
  if (imm_eseq_setup(&x->imm.eseq, &x->imm.seq))
    defer_return(error(DCP_ESEQABC));

  return 0;

defer:
  sequence_cleanup(x);
  return rc;
}

void sequence_cleanup(struct sequence *x)
{
  free((void *)x->name);
  free((void *)x->data);
  x->name = NULL;
  x->data = NULL;
  imm_eseq_cleanup(&x->imm.eseq);
}

struct sequence sequence_slice(struct sequence const *x, struct imm_range r)
{
  struct imm_seq seq = imm_seq_slice(&x->imm.seq, r);
  struct imm_eseq eseq = imm_eseq_slice(&x->imm.eseq, r);
  return (struct sequence){x->id, x->name, x->data, {seq, eseq}, NODE_INIT()};
}

int sequence_size(struct sequence const *x)
{
  return imm_seq_size(&x->imm.seq);
}
