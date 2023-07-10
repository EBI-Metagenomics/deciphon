#ifndef DECIPHON_U16TOA_H
#define DECIPHON_U16TOA_H

#include <stdint.h>

/* Convert uint16_t to null-terminated string.
 * Return the string length. */
static unsigned u16toa(char *str, uint16_t num)
{
  char const *const begin = str;
  *str = '0';

  unsigned denom = 10000;
  while (denom > 1 && !(num / denom))
    denom /= 10;

  while (denom)
  {
    *str = (char)('0' + (num / denom));
    num %= denom;
    ++str;
    denom /= 10;
  }
  *str = 0;
  return (unsigned)(str - begin);
}

#endif
