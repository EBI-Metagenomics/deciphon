#ifndef DECIPHON_OUCH_H
#define DECIPHON_OUCH_H

#include <stdio.h>

void ouch(char const *restrict fmt, ...) __attribute__((format(printf, 1, 2)));

#endif
