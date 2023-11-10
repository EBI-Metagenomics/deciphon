#include "debug.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void debug_print(int line, const char *src, char const *fmt, ...)
{
  va_list arg;
  va_start(arg, fmt);
  char const *s = getenv("DECIPHON_DEBUG");
  if (s && strcmp(s, "0"))
  {
    char location[256] = {0};
    snprintf(location, 256, "%s:%d", src, line);

    char message[256] = {0};
    vsnprintf(message, 256, fmt, arg);

    fprintf(stderr, "%s: %s\n", location, message);
  }
  va_end(arg);
}
