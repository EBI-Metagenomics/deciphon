#include "database_writer.h"
#include "database_version.h"
#include "deciphon.h"
#include "defer_return.h"
#include "error.h"
#include "fs.h"
#include "lio.h"
#include "magic_number.h"
#include "protein.h"
#include "read.h"
#include "write.h"
#include <unistd.h>

#define SPLIT_SIZE 4294967296 // 4GB

static void nullify_tempfiles(struct database_writer *x)
{
  lio_setup(&x->tmp.header, -1);
  lio_setup(&x->tmp.sizes, -1);
  for (int i = 0; i < DATABASE_WRITER_CHUNKS; ++i)
    lio_setup(x->tmp.proteins + i, -1);
}

static void destroy_tempfiles(struct database_writer *x)
{
  if (lio_file(&x->tmp.header) >= 0)
  {
    lio_flush(&x->tmp.header);
    fs_close(lio_file(&x->tmp.header));
  }
  if (lio_file(&x->tmp.sizes) >= 0)
  {
    lio_flush(&x->tmp.sizes);
    fs_close(lio_file(&x->tmp.sizes));
  }
  for (int i = 0; i < DATABASE_WRITER_CHUNKS; ++i)
  {
    if (lio_file(x->tmp.proteins + i) >= 0)
    {
      lio_flush(x->tmp.proteins + i);
      fs_close(lio_file(x->tmp.proteins + i));
    }
  }
  nullify_tempfiles(x);
}

static int create_tempfiles(struct database_writer *x)
{
  nullify_tempfiles(x);
  int rc = 0;

  int header = -1;
  int sizes = -1;

  if ((rc = fs_mkstemp(&header, ".header_XXXXXX"))) defer_return(error(rc));
  if ((rc = fs_mkstemp(&sizes, ".sizes_XXXXXX")))   defer_return(error(rc));
  lio_setup(&x->tmp.header, header);
  lio_setup(&x->tmp.sizes, sizes);

  for (int i = 0; i < DATABASE_WRITER_CHUNKS; ++i)
  {
    int proteins = -1;
    if ((rc = fs_mkstemp(&proteins, ".proteins_XXXXXX"))) defer_return(error(rc));
    lio_setup(x->tmp.proteins + i, proteins);
  }

  x->tmp.current_proteins = x->tmp.proteins;

  return 0;

defer:
  destroy_tempfiles(x);
  return rc;
}

static int pack_header_protein_sizes(struct database_writer *x)
{
  int rc = 0;
  if ((rc = write_array(&x->file, x->nproteins))) return error(rc);
  if ((rc = lio_flush(&x->tmp.sizes)))            return error_system(DCP_EFFLUSH, lio_syserror(rc));
  if ((rc = lio_rewind(&x->tmp.sizes)))           return error_system(DCP_EFWRITE, lio_syserror(rc));

  unsigned size = 0;
  struct lio_reader src = {0};
  lio_setup(&src, lio_file(&x->tmp.sizes));
  unsigned char *buf = NULL;
  while (!(rc = lio_read(&src, &buf)))
  {
    if (lio_free(&src, lip_unpack_int(buf, &size))) return error(DCP_EFDATA);
    if ((rc = write_int(&x->file, size)))           return error(rc);
  }
  return lio_eof(&src) ? 0 : error_system(DCP_EFREAD, lio_syserror(rc));
}

static int pack_header(struct database_writer *x)
{
  int rc = 0;
  struct lio_writer *src = &x->tmp.header;
  struct lio_writer *dst = &x->file;

  if ((rc = write_cstring(dst, "header"))) return error(rc);
  if ((rc = write_map(dst, 8)))            return error(rc);

  if ((rc = lio_rewind(src)))           return error_system(rc, lio_syserror(rc));
  if ((rc = lio_flush(dst)))            return error_system(DCP_EFFLUSH, lio_syserror(rc));
  if ((rc = fs_copy(dst->fd, src->fd))) return error(rc);

  if ((rc = write_cstring(dst, "has_ga"))) return error(rc);
  if ((rc = write_bool(dst, x->has_ga)))   return error(rc);

  if ((rc = write_cstring(dst, "protein_sizes"))) return error(rc);
  return error(pack_header_protein_sizes(x));
}

static int pack_proteins(struct database_writer *x)
{
  int rc = 0;
  if ((rc = write_cstring(&x->file, "proteins"))) return error(rc);
  if ((rc = write_array(&x->file, x->nproteins))) return error(rc);

  struct lio_writer *curr = x->tmp.proteins;
  struct lio_writer *last = x->tmp.current_proteins;
  while (curr <= last)
  {
    if ((rc = lio_rewind(curr)))                            return error_system(DCP_EFWRITE, lio_syserror(rc));
    if ((rc = lio_flush(curr)))                             return error_system(DCP_EFFLUSH, lio_syserror(rc));
    if ((rc = lio_flush(&x->file)))                         return error_system(DCP_EFFLUSH, lio_syserror(rc));
    if ((rc = fs_copy(lio_file(&x->file), lio_file(curr)))) return error(rc);
    fs_close(lio_file(curr));
    lio_setup(curr, -1);
    curr += 1;
  }
  return 0;
}

int database_writer_close(struct database_writer *db)
{
  int rc = 0;
  if ((rc = write_map(&db->file, 2))) defer_return(error(rc));
  if ((rc = pack_header(db)))         defer_return(error(rc));
  if ((rc = pack_proteins(db)))       defer_return(error(rc));
  if ((rc = lio_flush(&db->file)))    defer_return(error_system(DCP_EFFLUSH, lio_syserror(rc)));

  return 0;

defer:
  destroy_tempfiles(db);
  return rc;
}

void database_writer_init(struct database_writer *x, struct model_params params)
{
  x->nproteins = 0;
  x->params = params;
  x->has_ga = false;
}

int database_writer_open(struct database_writer *x, int fd)
{
  int rc = 0;

  x->nproteins = 0;
  lio_setup(&x->file, fd);
  if ((rc = create_tempfiles(x))) return error(rc);

  // the last header field is protein_sizes written when the file is closed
  struct model_params *p = &x->params;
  struct lio_writer *f = &x->tmp.header;

  if ((rc = write_cstring(f, "magic_number"))) defer_return(error(rc));
  if ((rc = write_int(f, MAGIC_NUMBER)))       defer_return(error(rc));

  if ((rc = write_cstring(f, "version")))    defer_return(error(rc));
  if ((rc = write_int(f, DATABASE_VERSION))) defer_return(error(rc));

  if ((rc = write_cstring(f, "entry_dist"))) defer_return(error(rc));
  if ((rc = write_int(f, p->entry_dist)))    defer_return(error(rc));

  if ((rc = write_cstring(f, "epsilon"))) defer_return(error(rc));
  if ((rc = write_float(f, p->epsilon)))  defer_return(error(rc));

  if ((rc = write_cstring(f, "abc")))          defer_return(error(rc));
  if (imm_abc_pack(&p->code->nuclt->super, f)) defer_return(error(DCP_EFWRITE));

  if ((rc = write_cstring(f, "amino")))  defer_return(error(rc));
  if (imm_abc_pack(&p->amino->super, f)) defer_return(error(DCP_EFWRITE));

  return 0;

defer:
  database_writer_close(x);
  return rc;
}

int database_writer_pack(struct database_writer *x,
                         struct protein const *protein)
{
  int rc = 0;

  long start = 0;
  if ((rc = lio_flush(x->tmp.current_proteins)))         return error_system(DCP_EFFLUSH, lio_syserror(rc));
  if ((rc = lio_wtell(x->tmp.current_proteins, &start))) return error_system(DCP_EFTELL, lio_syserror(rc));

  if (start >= SPLIT_SIZE)
  {
    x->tmp.current_proteins += 1;
    start = 0;
  }
  if (x->tmp.current_proteins >= x->tmp.proteins + DATABASE_WRITER_CHUNKS)
    return error(DCP_ELARGEFILE);

  if ((rc = protein_pack(protein, x->tmp.current_proteins))) return error(rc);

  long end = 0;
  if (lio_flush(x->tmp.current_proteins))      return error_system(DCP_EFFLUSH, lio_syserror(rc));
  if (lio_tell(x->tmp.current_proteins, &end)) return error_system(DCP_EFTELL, lio_syserror(rc));

  if ((end - start) > UINT_MAX) return error(DCP_ELARGEPROTEIN);

  unsigned protein_size = (unsigned)(end - start);
  if ((rc = write_int(&x->tmp.sizes, protein_size))) return error(rc);

  x->nproteins++;
  return 0;
}

void database_writer_set_has_ga(struct database_writer *x, bool has_ga)
{
  x->has_ga = has_ga;
}
