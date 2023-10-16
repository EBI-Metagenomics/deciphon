#include "seq.h"
#include "disambiguate.h"
#include "imm/imm.h"
#include "rc.h"
#include "seq_struct.h"
#include "strdup.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

int seq_setup(struct seq *x, long id, char const *name,
                  char const *data)
{
  x->id = id;
  x->name = name;

  char *new_data = NULL;
  if (!(new_data = dcp_strdup(data))) return DCP_ENOMEM;

  struct imm_abc const *abc = imm_eseq_abc(&x->imm_eseq);
  if (abc->typeid == IMM_DNA) disambiguate_dna(strlen(new_data), new_data);
  if (abc->typeid == IMM_RNA) disambiguate_rna(strlen(new_data), new_data);

  x->data = new_data;
  if (imm_seq_init(&x->imm_seq, imm_str(x->data), imm_eseq_abc(&x->imm_eseq)))
  {
    free((void *)new_data);
    return DCP_ESEQABC;
  }
  node_init(&x->node);
  return imm_eseq_setup(&x->imm_eseq, &x->imm_seq) ? DCP_ESEQABC : 0;
}

void seq_cleanup(struct seq *x)
{
  imm_eseq_cleanup(&x->imm_eseq);
  if (x->data)
  {
    free((void *)x->data);
    x->data = NULL;
  }
}

void seq_init(struct seq *x, struct imm_code const *code)
{
  x->id = 0;
  x->name = NULL;
  x->data = NULL;
  imm_eseq_init(&x->imm_eseq, code);
}

struct imm_seq const *seq_immseq(struct seq const *x)
{
  return &x->imm_seq;
}

struct imm_eseq const *seq_immeseq(struct seq const *x)
{
  return &x->imm_eseq;
}

long seq_id(struct seq const *x) { return x->id; }

int seq_size(struct seq const *x) { return imm_seq_size(&x->imm_seq); }

char const *seq_data(struct seq const *x)
{
  return x->imm_seq.str.data;
}

struct seq *seq_clone(struct seq *x)
{
  struct seq *seq = NULL;
  char const *name = NULL;

  if (!(seq = malloc(sizeof(*seq)))) goto cleanup;
  if (!(name = dcp_strdup(x->name))) goto cleanup;
  seq_init(seq, x->imm_eseq.code);
  int rc = seq_setup(seq, x->id, name, x->data);
  (void)rc;
  assert(!rc && "original sequence should be proper");

  return seq;

cleanup:
  if (seq) free(seq);
  if (name) free((void *)name);
  return NULL;
}

struct seq seq_slice(struct seq const *x, struct imm_range r)
{
  struct imm_seq seq = imm_seq_slice(&x->imm_seq, r);
  struct imm_eseq eseq = imm_eseq_slice(&x->imm_eseq, r);
  return (struct seq){x->id, x->name, imm_seq_data(&seq), seq, eseq, {}};
}
