#ifndef STATE_H
#define STATE_H

#include <stdbool.h>
#include "compiler_attributes.h"

#define STATE_ID_BITS 16

enum state_id
{
  STATE_M = (0 << (STATE_ID_BITS - 2)),
  STATE_I = (1 << (STATE_ID_BITS - 2)),
  STATE_D = (2 << (STATE_ID_BITS - 2)),
  STATE_X = (3 << (STATE_ID_BITS - 2)),
  STATE_F = (STATE_X | 0),
  STATE_R = (STATE_X | 1),
  STATE_G = (STATE_X | 2),
  STATE_S = (STATE_X | 3),
  STATE_N = (STATE_X | 4),
  STATE_B = (STATE_X | 5),
  STATE_E = (STATE_X | 6),
  STATE_J = (STATE_X | 7),
  STATE_C = (STATE_X | 8),
  STATE_T = (STATE_X | 9),
};

// Number of bits used by each core state
#define STATE_M_BITS 5 // (B, Mk, Ik, Dk) -> Mn: 2bits + (emis_size=5): 3bits
#define STATE_I_BITS 4 // (Mk, Ik)        -> Ik: 1bits + (emis_size=5): 3bits
#define STATE_D_BITS 1 // (Mk, Dk)        -> Dn: 1bit

// Number of bits used by each special state
#define STATE_S_BITS 0  // starting state
#define STATE_N_BITS 4  // (S, N)        -> N: 1bit + (emis_size=5): 3bits
#define STATE_B_BITS 2  // (S, N, E, J)  -> B: 2bits
#define STATE_E_BITS 15 // (Mk, Dk)      -> E: 1bit + (core_size=16,384): 14bits
#define STATE_C_BITS 4  // (E, C)        -> C: 1bit + (emis_size=5): 3bits
#define STATE_T_BITS 1  // (E, C)        -> T: 1bit
#define STATE_J_BITS 4  // (E, J)        -> J: 1bit + (emis_size=5): 3bits

int  state_make_end(void);
bool state_is_start(int id);
bool state_is_end(int id);
bool state_is_core(int id);
bool state_is_mute(int id);
int  state_core_idx(int id);
int  state_name(int id, char *name);
int  state_make_match_id(int idx);
int  state_make_insert_id(int idx);
int  state_make_delete_id(int idx);

__always_inline __attribute_const static int state_id_msb(int id) { return id & (3 << (STATE_ID_BITS - 2)); }

__always_inline __attribute_const static bool state_is_match(int id)  { return state_id_msb(id) == STATE_M; }
__always_inline __attribute_const static bool state_is_insert(int id) { return state_id_msb(id) == STATE_I; }
__always_inline __attribute_const static bool state_is_delete(int id) { return state_id_msb(id) == STATE_D; }

#endif
