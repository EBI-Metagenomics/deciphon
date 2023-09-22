#include "db_reader.h"
#include "defer_return.h"
#include "expect.h"
#include "imm/imm.h"
#include "lip/1darray/1darray.h"
#include "magic_number.h"
#include "rc.h"
#include "read.h"
#include <stdlib.h>

void dcp_db_reader_init(struct dcp_db_reader *x)
{
  x->nproteins = 0;
  x->protein_sizes = NULL;
}

static int unpack_header_protein_sizes(struct dcp_db_reader *x);

int dcp_db_reader_open(struct dcp_db_reader *x, FILE *fp)
{
  int rc = 0;

  x->nproteins = 0;
  x->protein_sizes = NULL;
  lip_file_init(&x->file, fp);

  if ((rc = read_mapsize(&x->file, 2))) defer_return(rc);

  if ((rc = read_key(&x->file, "header"))) defer_return(rc);
  if ((rc = read_mapsize(&x->file, 6))) defer_return(rc);

  unsigned magic_number = 0;
  if ((rc = read_key(&x->file, "magic_number"))) defer_return(rc);
  if ((rc = read_int(&x->file, &magic_number))) defer_return(rc);
  if (magic_number != MAGIC_NUMBER) defer_return(DCP_EFDATA);

  if ((rc = read_key(&x->file, "entry_dist"))) defer_return(rc);
  if ((rc = read_int(&x->file, &x->entry_dist))) defer_return(rc);
  if (!dcp_entry_dist_valid(x->entry_dist)) defer_return(DCP_EFDATA);

  if ((rc = read_key(&x->file, "epsilon"))) defer_return(rc);
  if ((rc = read_float(&x->file, &x->epsilon))) defer_return(rc);
  if (x->epsilon < 0 || x->epsilon > 1) defer_return(DCP_EFDATA);

  if ((rc = read_key(&x->file, "abc"))) defer_return(rc);
  if ((rc = read_abc(&x->file, &x->nuclt.super))) defer_return(rc);

  if ((rc = read_key(&x->file, "amino"))) defer_return(rc);
  if ((rc = read_abc(&x->file, &x->amino.super))) defer_return(rc);

  imm_nuclt_code_init(&x->code, &x->nuclt);
  if ((rc = read_key(&x->file, "protein_sizes"))) defer_return(rc);
  if ((rc = unpack_header_protein_sizes(x))) defer_return(rc);

  return rc;

defer:
  dcp_db_reader_close(x);
  return rc;
}

void dcp_db_reader_close(struct dcp_db_reader *x)
{
  if (x->protein_sizes) free(x->protein_sizes);
  x->protein_sizes = NULL;
}

static int unpack_header_protein_sizes(struct dcp_db_reader *x)
{
  enum lip_1darray_type type = 0;

  unsigned n = 0;
  if (!lip_read_1darray_size_type(&x->file, &n, &type)) return DCP_EFREAD;
  if (n > INT_MAX) return DCP_EFDATA;
  x->nproteins = (int)n;

  if (type != LIP_1DARRAY_UINT32) return DCP_EFDATA;

  x->protein_sizes = malloc(sizeof(*x->protein_sizes) * x->nproteins);
  if (!x->protein_sizes) return DCP_ENOMEM;

  if (!lip_read_1darray_u32_data(&x->file, x->nproteins, x->protein_sizes))
  {
    free(x->protein_sizes);
    x->protein_sizes = NULL;
    return DCP_EFREAD;
  }

  return 0;
}

struct dcp_model_params dcp_db_reader_params(struct dcp_db_reader const *x,
                                             struct imm_gencode const *gencode)
{
  return (struct dcp_model_params){gencode, &x->amino, &x->code, x->entry_dist,
                                   x->epsilon};
}
