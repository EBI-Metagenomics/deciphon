#ifndef ERROR_H
#define ERROR_H

#define error(x) error_raise((x), __LINE__, __FILE__, __func__)
int              error_raise(int error_code, int line, const char *file,
                             const char *func);

#endif
