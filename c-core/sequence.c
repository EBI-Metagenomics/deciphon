#include "sequence.h"
#include "defer_return.h"
#include "disambiguate.h"
#include "error.h"
#include "imm/abc.h"
#include "imm/dna.h"
#include "imm/rc.h"
#include "imm/rna.h"
#include "xstrdup.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

int sequence_init(struct sequence *x, struct imm_code const *code, long id,
                  char const *name, char const *data)
{
  int db_abc = code->abc->typeid;
  if (!(db_abc == IMM_DNA || db_abc == IMM_RNA))
    return error(DCP_ENUCLTNOSUPPORT);

  char *new_name = xstrdup(name);
  char *new_data = xstrdup(data);

  if (!new_name || !new_data)
  {
    free(new_name);
    free(new_data);
    return error(DCP_ENOMEM);
  }

  for (size_t i = 0; i < strlen(new_data); ++i)
    new_data[i] = toupper(new_data[i]);

  int rc = 0;
  x->id = id;
  x->name = new_name;
  x->data = new_data;
  imm_eseq_init(&x->imm.eseq, code);

  rc = disambiguate(strlen(x->data), x->data);
  if (rc) defer_return(rc);

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

  node_init(&x->node);
  if (imm_eseq_setup(&x->imm.eseq, &x->imm.seq) == IMM_ENOMEM)
    defer_return(error(DCP_ENOMEM));

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
