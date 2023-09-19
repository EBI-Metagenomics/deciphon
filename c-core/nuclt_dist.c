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
  if (!lip_write_array_size(file, 2)) return DCP_ENUCLTDPACK;
  if (imm_nuclt_lprob_pack(&x->nucltp, file)) return DCP_ENUCLTDPACK;
  if (imm_codon_marg_pack(&x->codonm, file)) return DCP_ENUCLTDPACK;
  return 0;
}

int dcp_nuclt_dist_unpack(struct dcp_nuclt_dist *x, struct lip_file *file)
{
  unsigned size = 0;
  if (!lip_read_array_size(file, &size)) return DCP_ENUCLTDUNPACK;
  if (size != 2) return DCP_ENUCLTDUNPACK;
  if (imm_nuclt_lprob_unpack(&x->nucltp, file)) return DCP_ENUCLTDUNPACK;
  if (imm_codon_marg_unpack(&x->codonm, file)) return DCP_ENUCLTDUNPACK;
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
