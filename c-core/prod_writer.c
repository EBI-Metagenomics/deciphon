#include "prod_writer.h"
#include "array_size.h"
#include "array_size_field.h"
#include "defer_return.h"
#include "format.h"
#include "fs.h"
#include "rc.h"
#include "strkcpy.h"

#define fmt(B, N, F, ...) dcp_format((B), (N), (F), __VA_ARGS__)
#define FMT(buf, format, ...) fmt((buf), array_size(buf), (format), __VA_ARGS__)

void dcp_prod_writer_init(struct dcp_prod_writer *x) { x->nthreads = 0; }

int dcp_prod_writer_open(struct dcp_prod_writer *x, int nthreads,
                         char const *dir)
{
  int rc = 0;

  size_t size = array_size_field(struct dcp_prod_writer, threads);
  if (nthreads > (int)size) defer_return(DCP_EMANYTHREADS);
  x->nthreads = nthreads;

  size = array_size_field(struct dcp_prod_writer, dirname);
  if (!strkcpy(x->dirname, dir, size)) defer_return(DCP_ELONGPATH);

  char hmmer_dir[DCP_PATH_MAX] = {0};
  if ((rc = FMT(hmmer_dir, "%s/hmmer", x->dirname))) return rc;

  if ((rc = dcp_fs_mkdir(x->dirname, true))) defer_return(rc);
  if ((rc = dcp_fs_mkdir(hmmer_dir, true))) defer_return(rc);

  for (int i = 0; i < nthreads; ++i)
  {
    if ((rc = dcp_prod_writer_thrd_init(x->threads + i, i, x->dirname)))
      defer_return(rc);
  }

  return rc;

defer:
  dcp_fs_rmdir(hmmer_dir);
  dcp_fs_rmdir(x->dirname);
  return rc;
}

int dcp_prod_writer_close(struct dcp_prod_writer *x)
{
  char filename[DCP_PATH_MAX] = {0};
  int rc = 0;

  if ((rc = FMT(filename, "%s/products.tsv", x->dirname))) return rc;

  FILE *fp = fopen(filename, "wb");
  if (!fp) return DCP_EFOPEN;

  bool ok = true;
  ok &= fputs("seq_id\tprofile\tabc\talt\t", fp) >= 0;
  ok &= fputs("null\tevalue\tmatch\n", fp) >= 0;
  if (!ok) defer_return(DCP_EWRITEPROD);

  for (int i = 0; i < x->nthreads; ++i)
  {
    char file[DCP_PATH_MAX] = {0};
    if ((rc = FMT(file, "%s/.products.%03d.tsv", x->dirname, i)))
      defer_return(rc);

    FILE *tmp = fopen(file, "rb");
    if (!tmp) defer_return(rc);

    if ((rc = dcp_fs_copy(fp, tmp)))
    {
      fclose(tmp);
      defer_return(rc);
    }

    if (fclose(tmp)) defer_return(DCP_EFCLOSE);

    if ((rc = dcp_fs_rmfile(file))) defer_return(rc);
  }

  return fclose(fp) ? DCP_EFCLOSE : 0;

defer:
  fclose(fp);
  return rc;
}

struct dcp_prod_writer_thrd *dcp_prod_writer_thrd(struct dcp_prod_writer *x,
                                                  int idx)
{
  return x->threads + idx;
}
