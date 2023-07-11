#include "format.h"
#include "rc.h"
#include <stdarg.h>
#include <stdio.h>

int dcp_format(char *dst, size_t dsize, char const *fmt, ...)
{
  va_list args = {0};
  va_start(args, fmt);
  int rc = vsnprintf(dst, dsize, fmt, args) < (int)dsize ? 0 : DCP_EFORMAT;
  va_end(args);
  return rc;
}