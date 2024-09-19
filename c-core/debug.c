#include "debug.h"
#include "loglevel.h"
#include <stdarg.h>
#include <stdio.h>

void debug_print(int line, const char *src, char const *fmt, ...)
{
  va_list arg;
  va_start(arg, fmt);
  if (loglevel() <= LOGLEVEL_DEBUG)
  {
    char location[256] = {0};
    snprintf(location, 256, "%s:%d", src, line);

    char message[256] = {0};
    vsnprintf(message, 256, fmt, arg);

    fprintf(stderr, "%s: %s\n", location, message);
  }
  va_end(arg);
}
