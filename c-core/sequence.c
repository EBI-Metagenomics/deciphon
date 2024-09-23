#include "sequence.h"
#include "defer_return.h"
#include "disambiguate.h"
#include "error.h"
#include "imm_abc.h"
#include "imm_dna.h"
#include "imm_rc.h"
#include "imm_rna.h"
#include "uppercase.h"
#include "xstrdup.h"
#include <stdlib.h>
#include <string.h>

int sequence_init(struct sequence *x, long id, char const *name,
                  char const *data)
{
  char *new_name = xstrdup(name);
  char *new_data = xstrdup(data);

  if (!new_name || !new_data)
  {
    free(new_name);
    free(new_data);
    return error(DCP_ENOMEM);
  }

  uppercase(strlen(new_data), new_data);

  x->id = id;
  x->name = new_name;
  x->data = new_data;
  x->encoded = false;

  int rc = disambiguate(strlen(x->data), x->data);
  if (rc)
  {
    free(new_name);
    free(new_data);
    return rc;
  }

  node_init(&x->node);
  return 0;
}

int sequence_encode(struct sequence *x, struct imm_code const *code)
{
  int rc = 0;
  if (x->encoded)
  {
    imm_eseq_cleanup(&x->imm.eseq);
    x->encoded = false;
  }

  int db_abc = code->abc->typeid;
  if (!(db_abc == IMM_DNA || db_abc == IMM_RNA))
    return error(DCP_ENUCLTNOSUPPORT);

  imm_eseq_init(&x->imm.eseq, code);

  if (imm_seq_init(&x->imm.seq, imm_str(x->data), imm_eseq_abc(&x->imm.eseq)))
  {
    struct imm_abc const *rna = &imm_rna_iupac.super.super;
    if (db_abc == IMM_DNA && !imm_seq_init(&x->imm.seq, imm_str(x->data), rna))
      defer_return(error(DCP_EDBDNASEQRNA));

    struct imm_abc const *dna = &imm_dna_iupac.super.super;
    if (db_abc == IMM_RNA && !imm_seq_init(&x->imm.seq, imm_str(x->data), dna))
      defer_return(error(DCP_EDBRNASEQDNA));

    defer_return(error(DCP_ESEQABC));
  }

  if (imm_eseq_setup(&x->imm.eseq, &x->imm.seq) == IMM_ENOMEM)
    defer_return(error(DCP_ENOMEM));

  x->encoded = true;
  return rc;

defer:
  imm_eseq_cleanup(&x->imm.eseq);
  return rc;
}

void sequence_cleanup(struct sequence *x)
{
  free((void *)x->name);
  free((void *)x->data);
  x->name = NULL;
  x->data = NULL;
  if (x->encoded)
  {
    imm_eseq_cleanup(&x->imm.eseq);
    x->encoded = false;
  }
}

struct sequence sequence_slice(struct sequence const *x, struct imm_range r)
{
  struct imm_seq seq = imm_seq_slice(&x->imm.seq, r);
  struct imm_eseq eseq = imm_eseq_slice(&x->imm.eseq, r);
  return (struct sequence){x->id,       x->name,    x->data,
                           {seq, eseq}, x->encoded, NODE_INIT()};
}

int sequence_size(struct sequence const *x)
{
  return imm_seq_size(&x->imm.seq);
}
