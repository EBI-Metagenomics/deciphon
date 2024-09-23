#include "product.h"
#include "defer_return.h"
#include "error.h"
#include "format.h"
#include "fs.h"
#include "xstrcpy.h"

void product_init(struct product *x)
{
  x->dirname[0] = '\0';
  x->closed = true;
}

int product_open(struct product *x, char const *dir)
{
  int rc = 0;
  char hmmer_dir[FS_PATH_MAX] = {0};

  if (xstrcpy(x->dirname, dir, FS_PATH_MAX))                         defer_return(error(DCP_ELONGPATH));
  if ((rc = format(hmmer_dir, FS_PATH_MAX, "%s/hmmer", x->dirname))) defer_return(rc);
  if ((rc = fs_mkdir(x->dirname, true)))                             defer_return(rc);
  if ((rc = fs_mkdir(hmmer_dir, true)))                              defer_return(rc);

  x->closed = false;
  return 0;

defer:
  fs_rmdir(hmmer_dir);
  fs_rmdir(x->dirname);
  return rc;
}

int product_close(struct product *x, int num_threads)
{
  if (x->closed) return 0;
  x->closed = true;

  char filename[FS_PATH_MAX] = {0};
  char *dir = x->dirname;
  int rc = 0;

  if ((rc = format(filename, FS_PATH_MAX, "%s/products.tsv", dir))) return rc;

  FILE *fp = fopen(filename, "wb");
  if (!fp) return error(DCP_EFOPEN);

  bool ok = true;
  ok &= fputs("sequence\t", fp)     >= 0;
  ok &= fputs("window\t", fp)       >= 0;
  ok &= fputs("window_start\t", fp) >= 0;
  ok &= fputs("window_stop\t", fp)  >= 0;
  ok &= fputs("hit\t", fp)          >= 0;
  ok &= fputs("hit_start\t", fp)    >= 0;
  ok &= fputs("hit_stop\t", fp)     >= 0;
  ok &= fputs("profile\t", fp)      >= 0;
  ok &= fputs("abc\t", fp)          >= 0;
  ok &= fputs("lrt\t", fp)          >= 0;
  ok &= fputs("evalue\t", fp)       >= 0;
  ok &= fputs("match\n", fp)        >= 0;
  if (!ok) defer_return(error(DCP_EWRITEPROD));

  for (int i = 0; i < num_threads; ++i)
  {
    char file[FS_PATH_MAX] = {0};
    if ((rc = format(file, FS_PATH_MAX, "%s/.products.%03d.tsv", dir, i)))
      defer_return(rc);

    FILE *tmp = fopen(file, "rb");
    if (!tmp) defer_return(rc);

    if ((rc = fs_fcopy(fp, tmp)))
    {
      fclose(tmp);
      defer_return(rc);
    }

    if (fclose(tmp)) defer_return(error(DCP_EFCLOSE));

    if ((rc = fs_rmfile(file))) defer_return(rc);
  }

  return fclose(fp) ? error(DCP_EFCLOSE) : 0;

defer:
  fclose(fp);
  return rc;
}
