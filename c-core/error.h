#ifndef ERROR_H
#define ERROR_H

#include <stddef.h>
#include <string.h>

int error_raise(int line, const char *file, const char *func, int error_code,
                const char *fmt, ...);

#define error(x)                  error_raise(__LINE__, __FILE__, __func__, (x), NULL)
#define error_detail(x, fmt, ...) error_raise(__LINE__, __FILE__, __func__, (x), ". Detail: " (fmt)                , __VA_ARGS__)
#define error_system(x, code)     error_raise(__LINE__, __FILE__, __func__, (x), (code > 0 ? ". System: %s" : NULL), strerror(code))

#endif
