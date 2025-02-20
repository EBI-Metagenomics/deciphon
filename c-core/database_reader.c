#include "database_reader.h"
#include "database_version.h"
#include "deciphon.h"
#include "defer_return.h"
#include "error.h"
#include "expect.h"
#include "fs.h"
#include "magic_number.h"
#include "read.h"
#include <errno.h>
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
  int fd = 0;

  if ((rc = fs_open(&fd, filename, O_RDONLY, 0644))) defer_return(error(rc));

  x->num_proteins = 0;
  x->protein_sizes = NULL;
  lio_setup(&x->file, fd);

  if ((rc = expect_map(&x->file, 2)))               defer_return(error(rc));

  if ((rc = expect_key(&x->file, "header")))        defer_return(error(rc));
  if ((rc = expect_map(&x->file, 8)))               defer_return(error(rc));

  int magic_number = 0;
  if ((rc = expect_key(&x->file, "magic_number")))  defer_return(error(rc));
  if ((rc = read_int(&x->file, &magic_number)))     defer_return(error(rc));
  if (magic_number != MAGIC_NUMBER)                 defer_return(error(DCP_ENOTDBFILE));

  int version = 0;
  if ((rc = expect_key(&x->file, "version")))       defer_return(error(rc));
  if ((rc = read_int(&x->file, &version)))          defer_return(error(rc));
  if (version != DATABASE_VERSION)                  defer_return(error(DCP_EDBVERSION));

  int entry_dist = 0;
  if ((rc = expect_key(&x->file, "entry_dist")))    defer_return(error(rc));
  if ((rc = read_int(&x->file, &entry_dist)))       defer_return(error(rc));
  x->entry_dist = (enum entry_dist)entry_dist;
  if (!entry_dist_valid(x->entry_dist))             defer_return(error(DCP_EFDATA));

  if ((rc = expect_key(&x->file, "epsilon")))       defer_return(error(rc));
  if ((rc = read_float(&x->file, &x->epsilon)))     defer_return(error(rc));
  if (x->epsilon < 0 || x->epsilon > 1)             defer_return(error(DCP_EFDATA));

  if ((rc = expect_key(&x->file, "abc")))           defer_return(error(rc));
  if (imm_abc_unpack(&x->nuclt.super, &x->file))    defer_return(error(DCP_ENUCLTDUNPACK));

  if ((rc = expect_key(&x->file, "amino")))         defer_return(error(rc));
  if (imm_abc_unpack(&x->amino.super, &x->file))    defer_return(error(DCP_ENUCLTDUNPACK));

  if ((rc = expect_key(&x->file, "has_ga")))        defer_return(error(rc));
  if ((rc = read_bool(&x->file, &x->has_ga)))       defer_return(error(rc));

  imm_nuclt_code_init(&x->code, &x->nuclt);
  if ((rc = expect_key(&x->file, "protein_sizes"))) defer_return(error(rc));
  if ((rc = unpack_header_protein_sizes(x)))        defer_return(error(rc));

  return 0;

defer:
  database_reader_close(x);
  return rc;
}

int database_reader_close(struct database_reader *x)
{
  int rc = 0;
  int fd = lio_rfile(&x->file);
  if (fd != -1) rc = error(fs_close(fd));
  lio_setup(&x->file, -1);
  return rc;
}

void database_reader_cleanup(struct database_reader *x)
{
  if (x->protein_sizes) free(x->protein_sizes);
  x->protein_sizes = NULL;
  database_reader_close(x);
  database_reader_init(x);
}

static int unpack_header_protein_sizes(struct database_reader *x)
{
  int rc = 0;
  uint32_t size = 0;

  if ((rc = read_array(&x->file, &size))) return error(rc);

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
