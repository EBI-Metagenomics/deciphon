#ifndef FORMAT_H
#define FORMAT_H

#include "compiler.h"
#include <stddef.h>

int format(char *dst, size_t size, char const *fmt, ...) FORMAT(3, 4);

#endif
