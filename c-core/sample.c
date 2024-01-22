#include "sample.h"
#include "array_size.h"
#include <stdlib.h>
#include <string.h>

float sample_float(void) { return (float)((double)rand() / (double)RAND_MAX); }

_Thread_local static char sequence[1001] = {0};

char const *sample_sequence(int size, char const *abc)
{
  if (size + 1 >= (int)array_size(sequence)) exit(1);

  for (int i = 0; i < size; ++i)
    sequence[i] = abc[rand() % strlen(abc)];

  sequence[size] = '\0';
  return sequence;
}
