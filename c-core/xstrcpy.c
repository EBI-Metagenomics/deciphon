#include "xstrcpy.h"

static inline size_t bsd_strlcpy(char *dst, char const *src, size_t dsize)
{
  char const *osrc = src;
  size_t nleft = dsize;

  if (nleft != 0)
  {
    while (--nleft != 0)
    {
      if ((*dst++ = *src++) == '\0') break;
    }
  }

  if (nleft == 0)
  {
    if (dsize != 0) *dst = '\0';
    while (*src++)
      ;
  }

  return (size_t)(src - osrc - 1);
}

bool xstrcpy(char *dst, char const *src, size_t dsize)
{
  return bsd_strlcpy(dst, src, dsize) < dsize;
}
