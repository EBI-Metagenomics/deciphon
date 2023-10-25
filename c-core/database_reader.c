#include "database_reader.h"
#include "defer_return.h"
#include "imm/imm.h"
#include "lip/1darray/1darray.h"
#include "magic_number.h"
#include "rc.h"
#include "unpack.h"
#include <stdlib.h>

void database_reader_init(struct database_reader *x)
{
  x->num_proteins = 0;
  x->protein_sizes = NULL;
  x->fp = NULL;
}

static int unpack_header_protein_sizes(struct database_reader *x);

int database_reader_open(struct database_reader *x, char const *filename)
{
  int rc = 0;
  if (!(x->fp = fopen(filename, "rb"))) defer_return(DCP_EOPENDB);

  x->num_proteins = 0;
  x->protein_sizes = NULL;
  lip_file_init(&x->file, x->fp);

  if ((rc = unpack_mapsize(&x->file, 2))) defer_return(rc);

  if ((rc = unpack_key(&x->file, "header"))) defer_return(rc);
  if ((rc = unpack_mapsize(&x->file, 6))) defer_return(rc);

  int magic_number = 0;
  if ((rc = unpack_key(&x->file, "magic_number"))) defer_return(rc);
  if ((rc = unpack_int(&x->file, &magic_number))) defer_return(rc);
  if (magic_number != MAGIC_NUMBER) defer_return(DCP_EFDATA);

  if ((rc = unpack_key(&x->file, "entry_dist"))) defer_return(rc);
  if ((rc = unpack_int(&x->file, &x->entry_dist))) defer_return(rc);
  if (!entry_dist_valid(x->entry_dist)) defer_return(DCP_EFDATA);

  if ((rc = unpack_key(&x->file, "epsilon"))) defer_return(rc);
  if ((rc = unpack_float(&x->file, &x->epsilon))) defer_return(rc);
  if (x->epsilon < 0 || x->epsilon > 1) defer_return(DCP_EFDATA);

  if ((rc = unpack_key(&x->file, "abc"))) defer_return(rc);
  if ((rc = unpack_abc(&x->file, &x->nuclt.super))) defer_return(rc);

  if ((rc = unpack_key(&x->file, "amino"))) defer_return(rc);
  if ((rc = unpack_abc(&x->file, &x->amino.super))) defer_return(rc);

  imm_nuclt_code_init(&x->code, &x->nuclt);
  if ((rc = unpack_key(&x->file, "protein_sizes"))) defer_return(rc);
  if ((rc = unpack_header_protein_sizes(x))) defer_return(rc);

  return rc;

defer:
  database_reader_close(x);
  return rc;
}

int database_reader_close(struct database_reader *x)
{
  if (x->protein_sizes) free(x->protein_sizes);
  x->protein_sizes = NULL;
  int rc = 0;
  if (x->fp) rc = fclose(x->fp) ? DCP_EFCLOSE : 0;
  x->fp = NULL;
  return rc;
}

static int unpack_header_protein_sizes(struct database_reader *x)
{
  enum lip_1darray_type type = 0;

  unsigned n = 0;
  if (!lip_read_1darray_size_type(&x->file, &n, &type)) return DCP_EFREAD;
  if (n > INT_MAX) return DCP_EFDATA;
  x->num_proteins = (int)n;

  if (type != LIP_1DARRAY_UINT32) return DCP_EFDATA;

  x->protein_sizes = malloc(sizeof(*x->protein_sizes) * x->num_proteins);
  if (!x->protein_sizes) return DCP_ENOMEM;

  if (!lip_read_1darray_u32_data(&x->file, x->num_proteins, x->protein_sizes))
  {
    free(x->protein_sizes);
    x->protein_sizes = NULL;
    return DCP_EFREAD;
  }

  return 0;
}

struct model_params database_reader_params(struct database_reader const *x,
                                           struct imm_gencode const *gencode)
{
  return (struct model_params){gencode, &x->amino, &x->code, x->entry_dist,
                               x->epsilon};
}
