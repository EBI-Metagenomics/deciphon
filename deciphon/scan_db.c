#include "scan_db.h"
#include "array_size_field.h"
#include "defer_return.h"
#include "strkcpy.h"

void scan_db_init(struct scan_db *x)
{
  x->filename[0] = 0;
  x->fp = NULL;
  dcp_db_reader_init(&x->db);
  dcp_protein_reader_init(&x->rdr);
}

int scan_db_open(struct scan_db *x, int nthreads)
{
  int rc = 0;

  if (!(x->fp = fopen(x->filename, "rb"))) defer_return(DCP_EOPENDB);
  if ((rc = dcp_db_reader_open(&x->db, x->fp))) defer_return(rc);
  if ((rc = dcp_protein_reader_setup(&x->rdr, &x->db, nthreads)))
    defer_return(rc);

  return 0;

defer:
  scan_db_close(x);
  return rc;
}

void scan_db_close(struct scan_db *x)
{
  dcp_db_reader_close(&x->db);
  if (x->fp)
  {
    fclose(x->fp);
    x->fp = NULL;
  }
}

int scan_db_set_filename(struct scan_db *x, char const *filename)
{
  size_t n = array_size_field(struct scan_db, filename);
  return strkcpy(x->filename, filename, n) ? 0 : DCP_ELONGPATH;
}

struct dcp_protein_reader *scan_db_reader(struct scan_db *x) { return &x->rdr; }

struct imm_abc const *scan_db_abc(struct scan_db const *x)
{
  return (struct imm_abc const *)&x->db.nuclt;
}
