#ifndef DECIPHON_STATE_H
#define DECIPHON_STATE_H

#include <stdbool.h>

enum
{
  STATE_ID_BITS = 16
};

enum dcp_state_id
{
  STATE_MATCH = (0 << (STATE_ID_BITS - 2)),
  STATE_INSERT = (1 << (STATE_ID_BITS - 2)),
  STATE_DELETE = (2 << (STATE_ID_BITS - 2)),
  STATE_EXT = (3 << (STATE_ID_BITS - 2)),
  STATE_R = (STATE_EXT | 0),
  STATE_S = (STATE_EXT | 1),
  STATE_N = (STATE_EXT | 2),
  STATE_B = (STATE_EXT | 3),
  STATE_E = (STATE_EXT | 4),
  STATE_J = (STATE_EXT | 5),
  STATE_C = (STATE_EXT | 6),
  STATE_T = (STATE_EXT | 7),
};

bool dcp_state_is_match(unsigned id);
bool dcp_state_is_insert(unsigned id);
bool dcp_state_is_delete(unsigned id);
bool dcp_state_is_mute(unsigned id);
unsigned dcp_state_idx(unsigned id);
char *dcp_state_name(unsigned id, char *name);

#endif