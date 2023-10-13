#include "state.h"
#include "model.h"
#include "u16toa.h"

static int id_msb(int id) { return id & (3 << (STATE_ID_BITS - 2)); }

int dcp_state_make_end(void) { return STATE_T; }

bool dcp_state_is_start(int id) { return id == STATE_S; }

bool dcp_state_is_end(int id) { return id == STATE_T; }

bool dcp_state_is_core(int id)
{
  return dcp_state_is_match(id) || dcp_state_is_delete(id) ||
         dcp_state_is_insert(id);
}

bool dcp_state_is_match(int id) { return id_msb(id) == STATE_MATCH; }

bool dcp_state_is_insert(int id) { return id_msb(id) == STATE_INSERT; }

bool dcp_state_is_delete(int id) { return id_msb(id) == STATE_DELETE; }

bool dcp_state_is_mute(int id)
{
  int msb = id_msb(id);
  return (msb == STATE_EXT) ? ((id == STATE_S || id == STATE_B ||
                                id == STATE_E || id == STATE_T))
                            : msb == STATE_DELETE;
}

int dcp_state_idx(int id) { return (id & (0xFFFF >> 2)) - 1; }

char *dcp_state_name(int id, char *name)
{
  int msb = id_msb(id);
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

int dcp_state_make_match_id(int idx) { return STATE_MATCH | (idx + 1); }

int dcp_state_make_insert_id(int idx) { return STATE_INSERT | (idx + 1); }

int dcp_state_make_delete_id(int idx) { return STATE_DELETE | (idx + 1); }
