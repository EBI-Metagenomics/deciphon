#ifndef DEBUG_H
#define DEBUG_H

#define debug(...) debug_print(__LINE__, __FILE__, __VA_ARGS__)

void debug_print(int line, const char *src, char const *fmt, ...);

#endif
