#ifndef ERROR_H
#define ERROR_H

#define error(x) error_debug((x), __LINE__, __FILE__)

int error_debug(int error_code, int line, const char *src);

#endif
