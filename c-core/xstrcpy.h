#ifndef XSTRCPY_H
#define XSTRCPY_H

#include <stddef.h>

// Return 0 on success; 1 otherwise.
int xstrcpy(char *dst, char const *src, size_t dsize);

#endif
