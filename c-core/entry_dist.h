#ifndef ENTRY_DIST_H
#define ENTRY_DIST_H

#include <stdbool.h>

enum entry_dist
{
  ENTRY_DIST_NULL,
  ENTRY_DIST_UNIFORM,
  ENTRY_DIST_OCCUPANCY,
};

static inline bool entry_dist_valid(int x)
{
  return x > ENTRY_DIST_NULL && x <= ENTRY_DIST_OCCUPANCY;
}

#endif
