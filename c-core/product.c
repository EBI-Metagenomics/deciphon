#include "product.h"
#include "deciphon.h"
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
  if ((rc = format(hmmer_dir, FS_PATH_MAX, "%s/hmmer", x->dirname))) defer_return(error(rc));
  if ((rc = fs_mkdir(x->dirname, true)))                             defer_return(error(rc));
  if ((rc = fs_mkdir(hmmer_dir, true)))                              defer_return(error(rc));

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

  if ((rc = format(filename, FS_PATH_MAX, "%s/products.tsv", dir))) return error(rc);

  FILE *fp = NULL;
  if ((rc = fs_fopen(&fp, filename, "wb"))) return error(rc);

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
      defer_return(error(rc));

    FILE *tmp = NULL;
    if ((rc = fs_fopen(&tmp, file, "rb"))) defer_return(error(rc));

    if ((rc = fs_fcopy(fp, tmp)))
    {
      fs_fclose(tmp);
      defer_return(error(rc));
    }

    if (fclose(tmp)) defer_return(error(DCP_EFCLOSE));

    if ((rc = fs_rmfile(file))) defer_return(error(rc));
  }

  return error(fs_fclose(fp));

defer:
  fs_fclose(fp);
  return rc;
}
