#ifndef DECIPHON_ENTRY_DIST_H
#define DECIPHON_ENTRY_DIST_H

#include <stdbool.h>

enum dcp_entry_dist
{
  DCP_ENTRY_DIST_NULL,
  DCP_ENTRY_DIST_UNIFORM,
  DCP_ENTRY_DIST_OCCUPANCY,
};

static inline bool dcp_entry_dist_valid(int x)
{
  return x > DCP_ENTRY_DIST_NULL && x <= DCP_ENTRY_DIST_OCCUPANCY;
}

#endif
