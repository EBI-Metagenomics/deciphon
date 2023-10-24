#ifndef VITERBI_DUMP_H
#define VITERBI_DUMP_H

#include <stdio.h>

struct protein;

void viterbi_dump(struct protein *, FILE *);
void viterbi_dump_dot(struct protein *, FILE *);

#endif
