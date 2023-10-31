#include "product.h"
#include "array_size.h"
#include "array_size_field.h"
#include "defer_return.h"
#include "format.h"
#include "fs.h"
#include "rc.h"
#include "xstrcpy.h"

void product_init(struct product *x)
{
  x->dirname[0] = '\0';
  x->num_threads = 0;
  x->closed = true;
}

int product_open(struct product *x, int num_threads, char const *dir)
{
  int rc = 0;

  int size = (int)array_size_field(struct product, threads);
  if (num_threads > size) defer_return(DCP_EMANYTHREADS);
  x->num_threads = num_threads;

  size = (int)array_size_field(struct product, dirname);
  if (!xstrcpy(x->dirname, dir, size)) defer_return(DCP_ELONGPATH);

  char hmmer_dir[DCP_PATH_MAX] = {0};
  if ((rc = format(hmmer_dir, DCP_PATH_MAX, "%s/hmmer", x->dirname))) return rc;

  if ((rc = fs_mkdir(x->dirname, true))) defer_return(rc);
  if ((rc = fs_mkdir(hmmer_dir, true))) defer_return(rc);

  for (int i = 0; i < num_threads; ++i)
  {
    if ((rc = product_thread_init(x->threads + i, i, x->dirname)))
      defer_return(rc);
  }

  x->closed = false;
  return 0;

defer:
  fs_rmdir(hmmer_dir);
  fs_rmdir(x->dirname);
  return rc;
}

int product_close(struct product *x)
{
  if (x->closed) return 0;
  x->closed = true;

  char filename[DCP_PATH_MAX] = {0};
  char *dir = x->dirname;
  int rc = 0;

  if ((rc = format(filename, DCP_PATH_MAX, "%s/products.tsv", dir))) return rc;

  FILE *fp = fopen(filename, "wb");
  if (!fp) return DCP_EFOPEN;

  bool ok = true;
  ok &= fputs("sequence\t", fp) >= 0;
  ok &= fputs("window\t", fp) >= 0;
  ok &= fputs("window_start\t", fp) >= 0;
  ok &= fputs("window_stop\t", fp) >= 0;
  ok &= fputs("profile\t", fp) >= 0;
  ok &= fputs("abc\t", fp) >= 0;
  ok &= fputs("lrt\t", fp) >= 0;
  ok &= fputs("evalue\t", fp) >= 0;
  ok &= fputs("match\n", fp) >= 0;
  if (!ok) defer_return(DCP_EWRITEPROD);

  for (int i = 0; i < x->num_threads; ++i)
  {
    char file[DCP_PATH_MAX] = {0};
    if ((rc = format(file, DCP_PATH_MAX, "%s/.products.%03d.tsv", dir, i)))
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

struct product_thread *product_thread(struct product *x, int tid)
{
  return x->threads + tid;
}
