#include "format.h"
#include "error.h"
#include "rc.h"
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>

int format(char *dst, size_t dsize, char const *fmt, ...)
{
  if (dsize > INT_MAX) return error(DCP_EINVALSIZE);
  va_list args = {0};
  va_start(args, fmt);
  int rc =
      vsnprintf(dst, dsize, fmt, args) < (int)dsize ? 0 : error(DCP_EFORMAT);
  va_end(args);
  return rc;
}
