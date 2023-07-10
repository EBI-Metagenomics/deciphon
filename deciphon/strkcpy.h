#ifndef DECIPHON_STRKCPY_H
#define DECIPHON_STRKCPY_H

#include "strlcpy.h"
#include <stdbool.h>
#include <stddef.h>

static inline bool strkcpy(char *dst, char const *src, size_t dsize)
{
  return dcp_strlcpy(dst, src, dsize) < dsize;
}

#endif
