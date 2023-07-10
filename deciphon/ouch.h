#ifndef DECIPHON_OUCH_H
#define DECIPHON_OUCH_H

#include "compiler.h"
#include "vfprintf.h"

dcp_template void ouch(char const *restrict fmt, ...)
    __attribute__((format(printf, 1, 2)));

dcp_template void ouch(char const *restrict fmt, ...)
{
  va_list params;
  va_start(params, fmt);
  dcp_vfprintf(stderr, fmt, params);
  va_end(params);
}

#endif
