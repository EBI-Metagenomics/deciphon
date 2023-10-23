#include "xu16toa.h"

int xu16toa(char *str, uint16_t num)
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
  return (int)(str - begin);
}