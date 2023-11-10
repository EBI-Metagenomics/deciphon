#include "error.h"
#include "string_error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int error_debug(int error_code, int line, const char *src)
{
  char const *s = getenv("DECIPHON_DEBUG");
  if (s && strcmp(s, "0"))
    fprintf(stderr, "%s:%d: %s\n", src, line, string_error(error_code));
  return error_code;
}
