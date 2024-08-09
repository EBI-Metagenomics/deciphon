#include "nuclt_dist.h"
#include "error.h"
#include "read.h"
#include "write.h"

void nuclt_dist_init(struct nuclt_dist *x, struct imm_nuclt const *nuclt)
{
  x->nucltp.nuclt = nuclt;
  x->codonm.nuclt = nuclt;
}

int nuclt_dist_pack(struct nuclt_dist const *x, struct lio_writer *file)
{
  int rc = 0;
  if ((rc = write_array(file, 2))) return rc;
  if (imm_nuclt_lprob_pack(&x->nucltp, file)) return rc;
  if (imm_codon_marg_pack(&x->codonm, file)) return rc;
  return 0;
}

int nuclt_dist_unpack(struct nuclt_dist *x, struct lio_reader *file)
{
  int rc = 0;
  uint32_t size = 0;
  if ((rc = read_array(file, &size))) return rc;
  if (size != 2) return error(DCP_ENUCLTDUNPACK);
  if (imm_nuclt_lprob_unpack(&x->nucltp, file)) return error(DCP_ENUCLTDUNPACK);
  if (imm_codon_marg_unpack(&x->codonm, file)) return error(DCP_ENUCLTDUNPACK);
  return 0;
}

void nuclt_dist_dump(struct nuclt_dist const *x, FILE *restrict fp)
{
  fprintf(fp, "nuclt_lprob");
  imm_nuclt_lprob_dump(&x->nucltp, fp);
  putc(' ', fp);

  fprintf(fp, "codon_marg");
  imm_codon_marg_dump(&x->codonm, fp);
}
