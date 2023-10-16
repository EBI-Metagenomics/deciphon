#ifndef U16TOA_H
#define U16TOA_H

#include <stdint.h>

/* Convert uint16_t to null-terminated string.
 * Return the string length. */
int dcp_u16toa(char *str, uint16_t num);

#endif
