#include "database_writer.h"
#include "database_version.h"
#include "defer_return.h"
#include "error.h"
#include "fs.h"
#include "lip/1darray/1darray.h"
#include "lip/file/read_int.h"
#include "lip/file/write_array.h"
#include "lip/file/write_bool.h"
#include "lip/file/write_cstr.h"
#include "lip/file/write_float.h"
#include "lip/file/write_int.h"
#include "magic_number.h"
#include "pack.h"
#include "protein.h"

static void nullify_tempfiles(struct database_writer *x)
{
  x->tmp.header.fp = NULL;
  x->tmp.sizes.fp = NULL;
  x->tmp.proteins.fp = NULL;
}

static void destroy_tempfiles(struct database_writer *x)
{
  if (x->tmp.header.fp) fclose(x->tmp.header.fp);
  if (x->tmp.sizes.fp) fclose(x->tmp.sizes.fp);
  if (x->tmp.proteins.fp) fclose(x->tmp.proteins.fp);
  nullify_tempfiles(x);
}

static int create_tempfiles(struct database_writer *x)
{
  nullify_tempfiles(x);
  int rc = 0;

  FILE **header = &x->tmp.header.fp;
  FILE **sizes = &x->tmp.sizes.fp;
  FILE **proteins = &x->tmp.proteins.fp;

  if ((rc = fs_mkstemp(header, ".header_XXXXXX"))) defer_return(rc);
  if ((rc = fs_mkstemp(sizes, ".sizes_XXXXXX"))) defer_return(rc);
  if ((rc = fs_mkstemp(proteins, ".proteins_XXXXXX"))) defer_return(rc);

  lip_file_init(&x->tmp.header, *header);
  lip_file_init(&x->tmp.sizes, *sizes);
  lip_file_init(&x->tmp.proteins, *proteins);

  return rc;

defer:
  destroy_tempfiles(x);
  return rc;
}

static int pack_header_protein_sizes(struct database_writer *db)
{
  enum lip_1darray_type type = LIP_1DARRAY_UINT32;

  if (!lip_write_1darray_size_type(&db->file, db->nproteins, type))
    return error(DCP_EFWRITE);

  rewind(lip_file_ptr(&db->tmp.sizes));

  unsigned size = 0;
  while (lip_read_int(&db->tmp.sizes, &size))
  {
    if (!lip_write_1darray_u32_item(&db->file, size)) return error(DCP_EFWRITE);
  }

  return feof(lip_file_ptr(&db->tmp.sizes)) ? 0 : error(DCP_EFWRITE);
}

static int pack_header(struct database_writer *db)
{
  int rc = 0;
  struct lip_file *stream = &db->file;

  if ((rc = pack_key(stream, "header"))) return rc;
  if ((rc = pack_mapsize(stream, 8))) return rc;

  FILE *src = lip_file_ptr(&db->tmp.header);
  FILE *dst = lip_file_ptr(stream);
  rewind(src);
  if ((rc = fs_copy(dst, src))) return rc;

  if ((rc = pack_key(stream, "has_ga"))) return rc;
  if ((rc = pack_bool(stream, db->has_ga))) return rc;

  if ((rc = pack_key(stream, "protein_sizes"))) return rc;
  return pack_header_protein_sizes(db);
}

static int pack_proteins(struct database_writer *db)
{
  if (!lip_write_cstr(&db->file, "proteins")) return error(DCP_EFWRITE);

  if (!lip_write_array_size(&db->file, db->nproteins))
    return error(DCP_EFWRITE);

  rewind(lip_file_ptr(&db->tmp.proteins));
  return fs_copy(lip_file_ptr(&db->file), lip_file_ptr(&db->tmp.proteins));
}

int database_writer_close(struct database_writer *db)
{
  int rc = 0;
  if ((rc = pack_mapsize(&db->file, 2))) defer_return(rc);

  if ((rc = pack_header(db))) defer_return(rc);
  if ((rc = pack_proteins(db))) defer_return(rc);

  return rc;

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

int database_writer_open(struct database_writer *x, FILE *restrict fp)
{
  int rc = 0;

  x->nproteins = 0;
  lip_file_init(&x->file, fp);
  if ((rc = create_tempfiles(x))) return rc;

  // the last header field is protein_sizes written when the file is closed
  struct model_params *p = &x->params;
  struct lip_file *stream = &x->tmp.header;

  if ((rc = pack_key(stream, "magic_number"))) defer_return(rc);
  if ((rc = pack_int(stream, MAGIC_NUMBER))) defer_return(rc);

  if ((rc = pack_key(stream, "version"))) defer_return(rc);
  if ((rc = pack_int(stream, DATABASE_VERSION))) defer_return(rc);

  if ((rc = pack_key(stream, "entry_dist"))) defer_return(rc);
  if ((rc = pack_int(stream, p->entry_dist))) defer_return(rc);

  if ((rc = pack_key(stream, "epsilon"))) defer_return(rc);
  if ((rc = pack_float(stream, p->epsilon))) defer_return(rc);

  if ((rc = pack_key(stream, "abc"))) defer_return(rc);
  if ((rc = pack_abc(stream, &p->code->nuclt->super))) defer_return(rc);

  if ((rc = pack_key(stream, "amino"))) defer_return(rc);
  if ((rc = pack_abc(stream, &p->amino->super))) defer_return(rc);

  return rc;

defer:
  database_writer_close(x);
  return rc;
}

int database_writer_pack(struct database_writer *x,
                         struct protein const *protein)
{
  int rc = 0;

  long start = 0;
  if ((rc = fs_tell(lip_file_ptr(&x->tmp.proteins), &start))) return rc;

  if ((rc = protein_pack(protein, &x->tmp.proteins))) return rc;

  long end = 0;
  if ((rc = fs_tell(lip_file_ptr(&x->tmp.proteins), &end))) return rc;

  if ((end - start) > UINT_MAX) return error(DCP_ELARGEPROTEIN);

  unsigned protein_size = (unsigned)(end - start);
  if ((rc = pack_int(&x->tmp.sizes, protein_size))) return rc;

  x->nproteins++;
  return rc;
}

void database_writer_set_has_ga(struct database_writer *x, bool has_ga)
{
  x->has_ga = has_ga;
}
