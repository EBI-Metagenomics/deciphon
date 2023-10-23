#include "xrealloc.h"
#include <stdlib.h>

void *xrealloc(void *ptr, size_t size)
{
  void *nptr = realloc(ptr, size);
  if (!nptr && ptr && size != 0) free(ptr);
  return nptr;
}
