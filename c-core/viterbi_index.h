#ifndef VITERBI_INDEX_H
#define VITERBI_INDEX_H

#include "compiler.h"
#include "state.h"

CONST int IX(int id, int core_index, int core_size)
{
  if (id == STATE_S) return 0;
  if (id == STATE_N) return 1;
  if (id == STATE_B) return 2;
  if (id == STATE_M) return 3 + core_index * 3 + 0;
  if (id == STATE_D) return 3 + core_index * 3 + 1;
  if (id == STATE_I) return 3 + core_index * 3 + 2;
  if (id == STATE_E) return 3 + core_size * 3 + 0;
  if (id == STATE_J) return 3 + core_size * 3 + 1;
  if (id == STATE_C) return 3 + core_size * 3 + 2;
  if (id == STATE_T) return 3 + core_size * 3 + 3;
  UNREACHABLE();
  return 0;
}

CONST int ID(int index, int core_size)
{
  if (index == IX(STATE_S, 0, core_size)) return STATE_S;
  if (index == IX(STATE_N, 0, core_size)) return STATE_N;
  if (index == IX(STATE_B, 0, core_size)) return STATE_B;
  if (index < IX(STATE_E, 0, core_size))
  {
    int core_index = (index - 3) / 3;
    if (index % 3 == 0) return state_make_match_id(core_index);
    if (index % 3 == 1) return state_make_delete_id(core_index);
    if (index % 3 == 2) return state_make_insert_id(core_index);
    UNREACHABLE();
  }
  if (index == IX(STATE_E, 0, core_size)) return STATE_E;
  if (index == IX(STATE_J, 0, core_size)) return STATE_J;
  if (index == IX(STATE_C, 0, core_size)) return STATE_C;
  if (index == IX(STATE_T, 0, core_size)) return STATE_T;
  UNREACHABLE();
  return 0;
}

CONST int SIX(void) { return IX(STATE_S, -1, -1); }
CONST int NIX(void) { return IX(STATE_N, -1, -1); }
CONST int BIX(void) { return IX(STATE_B, -1, -1); }

CONST int MIX(int core_index) { return IX(STATE_M, core_index, -1); }
CONST int DIX(int core_index) { return IX(STATE_D, core_index, -1); }
CONST int IIX(int core_index) { return IX(STATE_I, core_index, -1); }

CONST int EIX(int core_size) { return IX(STATE_E, -1, core_size); }
CONST int JIX(int core_size) { return IX(STATE_J, -1, core_size); }
CONST int CIX(int core_size) { return IX(STATE_C, -1, core_size); }
CONST int TIX(int core_size) { return IX(STATE_T, -1, core_size); }

#endif
