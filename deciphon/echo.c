#include "echo.h"
#include "vfprintf.h"
#include <stdarg.h>

void echo(char const *restrict fmt, ...)
{
  va_list params;
  va_start(params, fmt);
  dcp_vfprintf(stdout, fmt, params);
  va_end(params);
}
