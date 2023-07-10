#ifndef DECIPHON_ECHO_H
#define DECIPHON_ECHO_H

#include <stdio.h>

void echo(char const *restrict fmt, ...) __attribute__((format(printf, 1, 2)));

#endif
