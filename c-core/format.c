#include "format.h"
#include "error.h"
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>

int format(char *dst, size_t size, char const *fmt, ...)
{
  if (size > INT_MAX) return error(DCP_EINVALSIZE);
  va_list args = {0};
  va_start(args, fmt);
  int rc = vsnprintf(dst, size, fmt, args) < (int)size ? 0 : error(DCP_EFORMAT);
  va_end(args);
  return rc;
}
