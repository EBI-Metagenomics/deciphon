#include "state.h"
#include "model.h"
#include "u16toa.h"

static unsigned id_msb(unsigned id) { return id & (3U << (STATE_ID_BITS - 2)); }

bool dcp_state_is_match(unsigned id) { return id_msb(id) == STATE_MATCH; }

bool dcp_state_is_insert(unsigned id) { return id_msb(id) == STATE_INSERT; }

bool dcp_state_is_delete(unsigned id) { return id_msb(id) == STATE_DELETE; }

bool dcp_state_is_mute(unsigned id)
{
  unsigned msb = id_msb(id);
  return (msb == STATE_EXT) ? ((id == STATE_S || id == STATE_B ||
                                id == STATE_E || id == STATE_T))
                            : msb == STATE_DELETE;
}

unsigned dcp_state_idx(unsigned id) { return (id & (0xFFFF >> 2)) - 1; }

char *dcp_state_name(unsigned id, char *name)
{
  unsigned msb = id_msb(id);
  if (msb == STATE_EXT)
  {
    if (id == STATE_F)
      name[0] = 'F';
    else if (id == STATE_R)
      name[0] = 'R';
    else if (id == STATE_G)
      name[0] = 'G';
    else if (id == STATE_S)
      name[0] = 'S';
    else if (id == STATE_N)
      name[0] = 'N';
    else if (id == STATE_B)
      name[0] = 'B';
    else if (id == STATE_E)
      name[0] = 'E';
    else if (id == STATE_J)
      name[0] = 'J';
    else if (id == STATE_C)
      name[0] = 'C';
    else if (id == STATE_T)
      name[0] = 'T';
    name[1] = '\0';
    return name;
  }
  else
  {
    if (msb == STATE_MATCH)
      name[0] = 'M';
    else if (msb == STATE_INSERT)
      name[0] = 'I';
    else if (msb == STATE_DELETE)
      name[0] = 'D';
    dcp_u16toa(name + 1, (uint16_t)(dcp_state_idx(id) + 1));
    return name;
  }
}
