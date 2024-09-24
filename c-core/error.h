#ifndef ERROR_H
#define ERROR_H

#define error(x) error_print((x), __LINE__, __FILE__)
int              error_print(int error_code, int line, const char *src);

#endif
