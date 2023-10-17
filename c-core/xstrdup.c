#include "xstrdup.h"
#include <stdlib.h>
#include <string.h>

char *xstrdup(const char *s)
{
  size_t l = strlen(s);
  char *d = malloc(l + 1);
  if (!d) return NULL;
  return memcpy(d, s, l + 1);
}
