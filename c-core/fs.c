#if !defined(_POSIX_C_SOURCE) || _POSIX_C_SOURCE < 200809L
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#if !defined(_XOPEN_SOURCE) || _XOPEN_SOURCE < 700
#undef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#if !defined(_FILE_OFFSET_BITS) || _FILE_OFFSET_BITS < 64
#undef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

#include "fs.h"
#include "deciphon.h"
#include "error.h"
#include "xstrcpy.h"
#include <errno.h>
#include <ftw.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>

#if defined(__APPLE__)
#if defined(_DARWIN_C_SOURCE)
#undef _DARWIN_C_SOURCE
#endif
#define _DARWIN_C_SOURCE 1
#include <fcntl.h>
#include <sys/param.h>
#endif

#if defined(__APPLE__) || defined(__FreeBSD__)
#include <copyfile.h>
#else
#include <fcntl.h>
#include <sys/sendfile.h>
#endif

#define BUFFSIZE (8 * 1024)

int fs_seek(int fd, long offset, int whence)
{
  return lseek(fd, (off_t)offset, whence) < 0 ? error(DCP_EFSEEK) : 0;
}

static inline int full_write(int fd, size_t size, unsigned char const *buffer)
{
  while (size > 0)
  {
    ssize_t n = write(fd, buffer, size);
    if (n == -1) return -1;
    buffer += n;
    size -= (size_t)n;
  }
  return 0;
}

int fs_copy(int dst, int src)
{
  static _Thread_local unsigned char buffer[BUFFSIZE];
  ssize_t n = 0;
  while ((n = read(src, buffer, BUFFSIZE)) > 0)
  {
    if (full_write(dst, n, buffer)) return error(DCP_EFWRITE);
  }
  return n < 0 ? error(DCP_EFREAD) : 0;
}

int fs_fcopy(FILE *restrict dst, FILE *restrict src)
{
  static _Thread_local char buffer[BUFFSIZE];
  size_t n = 0;
  while ((n = fread(buffer, sizeof(*buffer), BUFFSIZE, src)) > 0)
  {
    if (n < BUFFSIZE && ferror(src)) return error(DCP_EFREAD);

    if (fwrite(buffer, sizeof(*buffer), n, dst) < n) return error(DCP_EFWRITE);
  }
  if (ferror(src)) return error(DCP_EFREAD);

  return 0;
}

static int getpath(int fd, unsigned size, char *filepath);

int fs_refopen(FILE *restrict fp, char const *mode, FILE **out)
{
  int fd = fileno(fp);
  if (fd < 0) return error(DCP_EGETPATH);
  char filepath[FILENAME_MAX] = {0};
  int rc = getpath(fd, sizeof filepath, filepath);
  if (rc) return rc;
  return (*out = fopen(filepath, mode)) ? 0 : error(DCP_EREFOPEN);
}

int fs_reopen(int fd, int mode, int *out)
{
  char filepath[FILENAME_MAX] = {0};
  int rc = getpath(fd, sizeof filepath, filepath);
  if (rc) return rc;
  return (*out = open(filepath, mode)) >= 0 ? 0 : error(DCP_EREFOPEN);
}

int fs_dup(int fd, int *out)
{
  return ((*out = dup(fd)) == -1 ? error(DCP_EDUP) : 0);
}

int getpath(int fd, unsigned size, char *filepath)
{
#ifdef __APPLE__
  (void)size;
  char pathbuf[MAXPATHLEN] = {0};
  if (fcntl(fd, F_GETPATH, pathbuf) < 0) return error(DCP_EGETPATH);
  if (strlen(pathbuf) >= size) return error(DCP_ETRUNCPATH);
  strcpy(filepath, pathbuf);
#else
  char pathbuf[FILENAME_MAX] = {0};
  sprintf(pathbuf, "/proc/self/fd/%d", fd);
  ssize_t n = readlink(pathbuf, filepath, size);
  if (n < 0) return error(DCP_EGETPATH);
  if (n >= size) return error(DCP_ETRUNCPATH);
  filepath[n] = '\0';
#endif

  return 0;
}

static int fletcher16(FILE *fp, uint8_t *buf, size_t bufsize, long *chk)
{
  size_t n = 0;
  uint16_t sum1 = 0;
  uint16_t sum2 = 0;
  while ((n = fread(buf, 1, bufsize, fp)) > 0)
  {
    if (n < bufsize && ferror(fp)) return error(DCP_EFREAD);
    for (int i = 0; i < (int)n; ++i)
    {
      sum1 = (sum1 + buf[i]) % 255;
      sum2 = (sum2 + sum1) % 255;
    }
  }
  if (ferror(fp)) return error(DCP_EFREAD);

  *chk = (sum2 << 8) | sum1;
  return 0;
}

int fs_chksum(char const *filepath, long *chk)
{
  static _Thread_local uint8_t buffer[8 * 1024];
  FILE *fp = fopen(filepath, "rb");
  if (!fp) return error(DCP_EFOPEN);

  int rc = fletcher16(fp, buffer, sizeof(buffer), chk);
  fclose(fp);
  return rc;
}

int fs_mkdir(char const *x, bool exist_ok)
{
  return (mkdir(x, 0755) && !(errno == EEXIST && exist_ok)) ? error(DCP_EMKDIR)
                                                            : 0;
}

int fs_rmdir(char const *x) { return rmdir(x) == 0 ? 0 : DCP_ERMDIR; }

int fs_rmfile(char const *x) { return unlink(x) == 0 ? 0 : DCP_ERMFILE; }

int fs_touch(char const *x)
{
  if (access(x, F_OK) == 0) return 0;
  FILE *fp = fopen(x, "wb");
  if (!fp) return error(DCP_EFOPEN);
  return fclose(fp) ? error(DCP_EFCLOSE) : 0;
}

static int unlink_callb(const char *path, const struct stat *stat, int flag,
                        struct FTW *buf)
{
  (void)stat;
  (void)flag;
  (void)buf;
  return remove(path);
}

int fs_rmtree(char const *dirpath)
{
  return nftw(dirpath, unlink_callb, 64, FTW_DEPTH | FTW_PHYS);
}

int fs_size(char const *filepath, long *size)
{
  struct stat st = {};
  if (stat(filepath, &st)) return error(DCP_EFSTAT);
  *size = (long)st.st_size;
  return 0;
}

int fs_mkstemp(int *fd, char const *template)
{
  char path[FS_PATH_MAX] = {0};
  if (xstrcpy(path, template, sizeof path)) return error(DCP_ENOMEM);

  if ((*fd = mkstemp(path)) < 0) return error_system(DCP_EMKSTEMP, errno);

  int rc = fs_rmfile(path);
  if (rc)
  {
    close(*fd);
    return error(rc);
  }

  return 0;
}
