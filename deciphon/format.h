#ifndef DECIPHON_FORMAT_H
#define DECIPHON_FORMAT_H

#include <stddef.h>

int dcp_format(char *dst, size_t dsize, char const *fmt, ...)
    __attribute__((format(printf, 3, 4)));

#endif
