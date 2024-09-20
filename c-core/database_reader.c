#include "database_reader.h"
#include "database_version.h"
#include "defer_return.h"
#include "error.h"
#include "expect.h"
#include "magic_number.h"
#include "read.h"
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void database_reader_init(struct database_reader *x)
{
  x->num_proteins = 0;
  lio_setup(&x->file, -1);
  x->protein_sizes = NULL;
  x->has_ga = false;
}

static int unpack_header_protein_sizes(struct database_reader *x);

int database_reader_open(struct database_reader *x, char const *filename)
{
  int rc = 0;
  int fd = -1;

  if ((fd = open(filename, O_RDONLY, 0644)) < 0)
    defer_return(error(DCP_EOPENDB));

  x->num_proteins = 0;
  x->protein_sizes = NULL;
  lio_setup(&x->file, fd);

  if ((rc = expect_map(&x->file, 2))) defer_return(rc);

  if ((rc = expect_key(&x->file, "header"))) defer_return(rc);
  if ((rc = expect_map(&x->file, 8))) defer_return(rc);

  int magic_number = 0;
  if ((rc = expect_key(&x->file, "magic_number"))) defer_return(rc);
  if ((rc = read_int(&x->file, &magic_number))) defer_return(rc);
  if (magic_number != MAGIC_NUMBER) defer_return(error(DCP_ENOTDBFILE));

  int version = 0;
  if ((rc = expect_key(&x->file, "version"))) defer_return(rc);
  if ((rc = read_int(&x->file, &version))) defer_return(rc);
  if (version != DATABASE_VERSION) defer_return(error(DCP_EDBVERSION));

  int entry_dist = 0;
  if ((rc = expect_key(&x->file, "entry_dist"))) defer_return(rc);
  if ((rc = read_int(&x->file, &entry_dist))) defer_return(rc);
  x->entry_dist = (enum entry_dist)entry_dist;
  if (!entry_dist_valid(x->entry_dist)) defer_return(error(DCP_EFDATA));

  if ((rc = expect_key(&x->file, "epsilon"))) defer_return(rc);
  if ((rc = read_float(&x->file, &x->epsilon))) defer_return(rc);
  if (x->epsilon < 0 || x->epsilon > 1) defer_return(error(DCP_EFDATA));

  if ((rc = expect_key(&x->file, "abc"))) defer_return(rc);
  if (imm_abc_unpack(&x->nuclt.super, &x->file)) defer_return(DCP_EFREAD);

  if ((rc = expect_key(&x->file, "amino"))) defer_return(rc);
  if (imm_abc_unpack(&x->amino.super, &x->file)) defer_return(DCP_EFREAD);

  if ((rc = expect_key(&x->file, "has_ga"))) defer_return(rc);
  if ((rc = read_bool(&x->file, &x->has_ga))) defer_return(rc);

  imm_nuclt_code_init(&x->code, &x->nuclt);
  if ((rc = expect_key(&x->file, "protein_sizes"))) defer_return(rc);
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
  int fd = lio_rfile(&x->file);
  if (fd != -1) rc = close(fd) ? error(DCP_EFCLOSE) : 0;
  database_reader_init(x);
  return rc;
}

static int unpack_header_protein_sizes(struct database_reader *x)
{
  int rc = 0;
  uint32_t size = 0;

  if ((rc = read_array(&x->file, &size))) return rc;

  if (size > INT_MAX) return error(DCP_EFDATA);
  x->num_proteins = (int)size;

  x->protein_sizes = malloc(sizeof(*x->protein_sizes) * x->num_proteins);
  if (!x->protein_sizes) return error(DCP_ENOMEM);

  for (int i = 0; i < x->num_proteins; ++i)
  {
    if ((rc = read_int(&x->file, x->protein_sizes + i)))
    {
      free(x->protein_sizes);
      x->protein_sizes = NULL;
      return error(rc);
    }
  }

  return 0;
}

struct model_params database_reader_params(struct database_reader const *x,
                                           struct imm_gencode const *gencode)
{
  return (struct model_params){gencode, &x->amino, &x->code, x->entry_dist,
                               x->epsilon};
}
