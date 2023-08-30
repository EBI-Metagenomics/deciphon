#include "nuclt_dist.h"
#include "lip/lip.h"
#include "rc.h"

void dcp_nuclt_dist_init(struct dcp_nuclt_dist *x,
                         struct imm_nuclt const *nuclt)
{
  x->nucltp.nuclt = nuclt;
  x->codonm.nuclt = nuclt;
}

int dcp_nuclt_dist_pack(struct dcp_nuclt_dist const *x, struct lip_file *file)
{
  int rc = DCP_ENUCLTDPACK;
  lip_write_array_size(file, 2);
  if (imm_nuclt_lprob_pack(&x->nucltp, file)) return rc;
  if (imm_codon_marg_pack(&x->codonm, file)) return rc;
  return 0;
}

int dcp_nuclt_dist_unpack(struct dcp_nuclt_dist *x, struct lip_file *file)
{
  int rc = DCP_ENUCLTDUNPACK;
  unsigned size = 0;
  lip_read_array_size(file, &size);
  assert(size == 2);
  if (imm_nuclt_lprob_unpack(&x->nucltp, file)) return rc;
  if (imm_codon_marg_unpack(&x->codonm, file)) return rc;
  return 0;
}

void dcp_nuclt_dist_dump(struct dcp_nuclt_dist const *x, FILE *restrict fp)
{
  fprintf(fp, "nuclt_lprob");
  imm_nuclt_lprob_dump(&x->nucltp, fp);
  putc(' ', fp);

  fprintf(fp, "codon_marg");
  imm_codon_marg_dump(&x->codonm, fp);
}
