#ifndef DECIPHON_ECHO_H
#define DECIPHON_ECHO_H

#include "compiler.h"
#include "vfprintf.h"

dcp_template void echo(char const *restrict fmt, ...)
    __attribute__((format(printf, 1, 2)));

dcp_template void echo(char const *restrict fmt, ...)
{
  va_list params;
  va_start(params, fmt);
  dcp_vfprintf(stdout, fmt, params);
  va_end(params);
}

#endif
