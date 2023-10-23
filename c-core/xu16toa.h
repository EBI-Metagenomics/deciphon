#ifndef XU16TOA_H
#define XU16TOA_H

#include <stdint.h>

/* Convert uint16_t to null-terminated string.
 * Return the string length. */
int xu16toa(char *str, uint16_t num);

#endif
