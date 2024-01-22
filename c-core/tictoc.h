#ifndef TICTOC_H
#define TICTOC_H

#include <stdio.h>
#include <time.h>

_Thread_local static struct timespec tic_time = {};
_Thread_local static struct timespec toc_time = {};

static void tic(void)
{
  if (clock_gettime(CLOCK_MONOTONIC, &tic_time)) perror("failed to tic");
}

static void toc(char const *name)
{
  if (clock_gettime(CLOCK_MONOTONIC, &toc_time)) perror("failed to toc");

  double elapsed = 1000.0 * toc_time.tv_sec + 1e-6 * toc_time.tv_nsec -
                   (1000.0 * tic_time.tv_sec + 1e-6 * tic_time.tv_nsec);

  fprintf(stdout, "%s: %ldms\n", name, (long)elapsed);
}

#endif
