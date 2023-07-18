#include "seq.h"
#include "imm/imm.h"
#include "rc.h"
#include "seq_struct.h"
#include <stddef.h>
#include <stdlib.h>

int dcp_seq_setup(struct dcp_seq *x, long id, char const *name,
                  char const *data)
{
  x->id = id;
  x->name = name;
  x->data = data;
  x->imm_seq = imm_seq(imm_str(x->data), imm_eseq_abc(&x->imm_eseq));
  return imm_eseq_setup(&x->imm_eseq, &x->imm_seq) ? DCP_ESEQABC : 0;
}

void dcp_seq_cleanup(struct dcp_seq *x) { imm_eseq_cleanup(&x->imm_eseq); }

void dcp_seq_init(struct dcp_seq *x, struct imm_code const *code)
{
  x->id = 0;
  x->name = NULL;
  x->data = NULL;
  imm_eseq_init(&x->imm_eseq, code);
}

struct imm_seq const *dcp_seq_immseq(struct dcp_seq const *x)
{
  return &x->imm_seq;
}

struct imm_eseq const *dcp_seq_immeseq(struct dcp_seq const *x)
{
  return &x->imm_eseq;
}

long dcp_seq_id(struct dcp_seq const *x) { return x->id; }

unsigned dcp_seq_size(struct dcp_seq const *x)
{
  return imm_seq_size(&x->imm_seq);
}

char const *dcp_seq_data(struct dcp_seq const *x)
{
  return imm_seq_str(&x->imm_seq);
}
