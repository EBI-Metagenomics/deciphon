#include "db_writer.h"
#include "defer_return.h"
#include "fs.h"
#include "lip/1darray/1darray.h"
#include "magic_number.h"
#include "protein.h"
#include "rc.h"

static int pack_entry_dist(struct lip_file *file,
                           enum dcp_entry_dist const *edist)
{
  if (!lip_write_cstr(file, "entry_dist")) return DCP_EFWRITE;
  if (!lip_write_int(file, *edist)) return DCP_EFWRITE;
  return 0;
}

static int pack_epsilon(struct lip_file *file, float const *epsilon)
{
  if (!lip_write_cstr(file, "epsilon")) return DCP_EFWRITE;
  if (!lip_write_float(file, *epsilon)) return DCP_EFWRITE;
  return 0;
}

static int pack_nuclt(struct lip_file *file, struct imm_nuclt const *nuclt)
{
  if (!lip_write_cstr(file, "abc")) return DCP_EFWRITE;
  if (imm_abc_pack(&nuclt->super, file)) return DCP_EFWRITE;
  return 0;
}

static int pack_amino(struct lip_file *file, struct imm_amino const *amino)
{
  if (!lip_write_cstr(file, "amino")) return DCP_EFWRITE;
  if (imm_abc_pack(&amino->super, file)) return DCP_EFWRITE;
  return 0;
}

static void destroy_tempfiles(struct dcp_db_writer *db)
{
  if (db->tmp.header.fp) fclose(db->tmp.header.fp);
  if (db->tmp.prot_sizes.fp) fclose(db->tmp.prot_sizes.fp);
  if (db->tmp.proteins.fp) fclose(db->tmp.proteins.fp);
}

static int create_tempfiles(struct dcp_db_writer *db)
{
  lip_file_init(&db->tmp.header, tmpfile());
  lip_file_init(&db->tmp.prot_sizes, tmpfile());
  lip_file_init(&db->tmp.proteins, tmpfile());

  int rc = 0;
  if (!db->tmp.header.fp || !db->tmp.prot_sizes.fp || !db->tmp.proteins.fp)
    defer_return(DCP_EOPENTMP);

  return rc;

defer:
  destroy_tempfiles(db);
  return rc;
}

static int db_writer_pack_magic_number(struct dcp_db_writer *db)
{
  if (!lip_write_cstr(&db->tmp.header, "magic_number")) return DCP_EFWRITE;

  if (!lip_write_int(&db->tmp.header, MAGIC_NUMBER)) return DCP_EFWRITE;

  db->header_size++;
  return 0;
}

static int db_writer_pack_float_size(struct dcp_db_writer *db)
{
  if (!lip_write_cstr(&db->tmp.header, "float_size")) return DCP_EFWRITE;

  // unsigned size = IMM_FLOAT_BYTES;
  unsigned size = 4;
  assert(size == 4 || size == 8);
  if (!lip_write_int(&db->tmp.header, size)) return DCP_EFWRITE;

  db->header_size++;
  return 0;
}

static int pack_protein(struct lip_file *file, void const *protein)
{
  return dcp_protein_pack(protein, file);
}

static int db_writer_pack_prof(struct dcp_db_writer *db, void const *arg)
{
  int rc = 0;

  long start = 0;
  if ((rc = dcp_fs_tell(lip_file_ptr(&db->tmp.proteins), &start))) return rc;

  if ((rc = pack_protein(&db->tmp.proteins, arg))) return rc;

  long end = 0;
  if ((rc = dcp_fs_tell(lip_file_ptr(&db->tmp.proteins), &end))) return rc;

  if ((end - start) > UINT_MAX) return DCP_ELARGEPROTEIN;

  unsigned prot_size = (unsigned)(end - start);
  if (!lip_write_int(&db->tmp.prot_sizes, prot_size)) return DCP_EFWRITE;

  db->nproteins++;
  return rc;
}

static int pack_header_prot_sizes(struct dcp_db_writer *db)
{
  enum lip_1darray_type type = LIP_1DARRAY_UINT32;

  if (!lip_write_1darray_size_type(&db->file, db->nproteins, type))
    return DCP_EFWRITE;

  rewind(lip_file_ptr(&db->tmp.prot_sizes));

  unsigned size = 0;
  while (lip_read_int(&db->tmp.prot_sizes, &size))
    lip_write_1darray_u32_item(&db->file, size);

  if (!feof(lip_file_ptr(&db->tmp.prot_sizes))) return DCP_EFWRITE;

  return 0;
}

static int pack_header(struct dcp_db_writer *db)
{
  struct lip_file *file = &db->file;
  if (!lip_write_cstr(file, "header")) return DCP_EFWRITE;

  if (!lip_write_map_size(file, db->header_size + 1)) return DCP_EFWRITE;

  rewind(lip_file_ptr(&db->tmp.header));
  int rc = dcp_fs_copy(lip_file_ptr(file), lip_file_ptr(&db->tmp.header));
  if (rc) return rc;

  if (!lip_write_cstr(file, "protein_sizes")) return DCP_EFWRITE;
  return pack_header_prot_sizes(db);
}

static int pack_proteins(struct dcp_db_writer *db)
{
  if (!lip_write_cstr(&db->file, "proteins")) return DCP_EFWRITE;

  if (!lip_write_array_size(&db->file, db->nproteins)) return DCP_EFWRITE;

  rewind(lip_file_ptr(&db->tmp.proteins));
  return dcp_fs_copy(lip_file_ptr(&db->file), lip_file_ptr(&db->tmp.proteins));
}

int dcp_db_writer_close(struct dcp_db_writer *db)
{
  int rc = 0;
  if (!lip_write_map_size(&db->file, 2)) defer_return(DCP_EFWRITE);

  if ((rc = pack_header(db))) defer_return(rc);

  if ((rc = pack_proteins(db))) defer_return(rc);

  return rc;

defer:
  destroy_tempfiles(db);
  return rc;
}

void dcp_db_writer_init(struct dcp_db_writer *x, struct dcp_model_params params)
{
  x->params = params;
}

int dcp_db_writer_open(struct dcp_db_writer *x, FILE *fp)
{
  int rc = 0;

  x->nproteins = 0;
  x->header_size = 0;
  lip_file_init(&x->file, fp);
  if ((rc = create_tempfiles(x))) return rc;

  struct dcp_model_params *p = &x->params;
  if ((rc = db_writer_pack_magic_number(x))) defer_return(rc);
  if ((rc = db_writer_pack_float_size(x))) defer_return(rc);
  if ((rc = pack_entry_dist(&x->tmp.header, &p->entry_dist))) defer_return(rc);
  if ((rc = pack_epsilon(&x->tmp.header, &p->epsilon))) defer_return(rc);
  if ((rc = pack_nuclt(&x->tmp.header, p->code->nuclt))) defer_return(rc);
  if ((rc = pack_amino(&x->tmp.header, p->amino))) defer_return(rc);
  x->header_size += 4;

  return rc;

defer:
  dcp_db_writer_close(x);
  return rc;
}

int dcp_db_writer_pack(struct dcp_db_writer *db,
                       struct dcp_protein const *protein)
{
  return db_writer_pack_prof(db, protein);
}