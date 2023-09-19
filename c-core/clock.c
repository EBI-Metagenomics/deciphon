#include "clock.h"
#include <assert.h>

long dcp_clock(void)
{
  struct timespec time = {};
  if (clock_gettime(CLOCK_MONOTONIC, &time) == -1)
  {
    perror("clock_gettime");
    assert(false);
  }
  return time.tv_sec * 1e3 + time.tv_nsec / 1e6;
}
