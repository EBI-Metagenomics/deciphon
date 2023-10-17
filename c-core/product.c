#include "product.h"
#include "array_size.h"
#include "array_size_field.h"
#include "defer_return.h"
#include "format.h"
#include "fs.h"
#include "rc.h"
#include "xstrcpy.h"

#define fmt(B, N, F, ...) dcp_format((B), (N), (F), __VA_ARGS__)
#define FMT(buf, format, ...) fmt((buf), array_size(buf), (format), __VA_ARGS__)

void product_init(struct product *x) { x->nthreads = 0; }

int product_open(struct product *x, int nthreads, char const *dir)
{
  int rc = 0;

  size_t size = array_size_field(struct product, threads);
  if (nthreads > (int)size) defer_return(DCP_EMANYTHREADS);
  x->nthreads = nthreads;

  size = array_size_field(struct product, dirname);
  if (!xstrcpy(x->dirname, dir, size)) defer_return(DCP_ELONGPATH);

  char hmmer_dir[DCP_PATH_MAX] = {0};
  if ((rc = FMT(hmmer_dir, "%s/hmmer", x->dirname))) return rc;

  if ((rc = fs_mkdir(x->dirname, true))) defer_return(rc);
  if ((rc = fs_mkdir(hmmer_dir, true))) defer_return(rc);

  for (int i = 0; i < nthreads; ++i)
  {
    if ((rc = product_thread_init(x->threads + i, i, x->dirname)))
      defer_return(rc);
  }

  return rc;

defer:
  fs_rmdir(hmmer_dir);
  fs_rmdir(x->dirname);
  return rc;
}

int product_close(struct product *x)
{
  char filename[DCP_PATH_MAX] = {0};
  int rc = 0;

  if ((rc = FMT(filename, "%s/products.tsv", x->dirname))) return rc;

  FILE *fp = fopen(filename, "wb");
  if (!fp) return DCP_EFOPEN;

  bool ok = true;
  ok &= fputs("sequence\twindow\twindow_start\twindow_stop\t", fp) >= 0;
  ok &= fputs("profile\tabc\tlrt\tevalue\tmatch\n", fp) >= 0;
  if (!ok) defer_return(DCP_EWRITEPROD);

  for (int i = 0; i < x->nthreads; ++i)
  {
    char file[DCP_PATH_MAX] = {0};
    if ((rc = FMT(file, "%s/.products.%03d.tsv", x->dirname, i)))
      defer_return(rc);

    FILE *tmp = fopen(file, "rb");
    if (!tmp) defer_return(rc);

    if ((rc = fs_copy(fp, tmp)))
    {
      fclose(tmp);
      defer_return(rc);
    }

    if (fclose(tmp)) defer_return(DCP_EFCLOSE);

    if ((rc = fs_rmfile(file))) defer_return(rc);
  }

  return fclose(fp) ? DCP_EFCLOSE : 0;

defer:
  fclose(fp);
  return rc;
}

struct product_thread *product_thread(struct product *x, int idx)
{
  return x->threads + idx;
}
