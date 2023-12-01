#include "state.h"
#include "xu16toa.h"
#include <assert.h>

static inline int id_msb(int id) { return id & (3 << (STATE_ID_BITS - 2)); }

int state_make_end(void) { return STATE_T; }

bool state_is_start(int id) { return id == STATE_S; }

bool state_is_end(int id) { return id == STATE_T; }

bool state_is_core(int id)
{
  return state_is_match(id) || state_is_delete(id) || state_is_insert(id);
}

bool state_is_match(int id) { return id_msb(id) == STATE_M; }

bool state_is_insert(int id) { return id_msb(id) == STATE_I; }

bool state_is_delete(int id) { return id_msb(id) == STATE_D; }

bool state_is_mute(int id)
{
  int msb = id_msb(id);
  return (msb == STATE_X) ? ((id == STATE_S || id == STATE_B || id == STATE_E ||
                              id == STATE_T))
                          : msb == STATE_D;
}

int state_core_idx(int id) { return (id & (0xFFFF >> 2)) - 1; }

char *state_name(int id, char *name)
{
  int msb = id_msb(id);
  if (msb == STATE_X)
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
    else
      assert(0);
    name[1] = '\0';
    return name;
  }
  else
  {
    if (msb == STATE_M)
      name[0] = 'M';
    else if (msb == STATE_I)
      name[0] = 'I';
    else if (msb == STATE_D)
      name[0] = 'D';
    else
      assert(0);
    xu16toa(name + 1, (uint16_t)(state_core_idx(id) + 1));
    return name;
  }
}

int state_make_match_id(int idx) { return STATE_M | (idx + 1); }

int state_make_insert_id(int idx) { return STATE_I | (idx + 1); }

int state_make_delete_id(int idx) { return STATE_D | (idx + 1); }
