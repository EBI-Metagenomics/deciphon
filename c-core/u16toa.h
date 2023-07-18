#ifndef DECIPHON_U16TOA_H
#define DECIPHON_U16TOA_H

#include <stdint.h>

/* Convert uint16_t to null-terminated string.
 * Return the string length. */
unsigned dcp_u16toa(char *str, uint16_t num);

#endif
