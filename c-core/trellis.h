#ifndef DECIPHON_TRELLIS_H
#define DECIPHON_TRELLIS_H

#include "bit.h"
#include "compiler.h"
#include "rc.h"
#include "reallocf.h"
#include "state.h"
#include "static_assert.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// Number of bits used by each special state
#define SBITS 0  // starting state
#define NBITS 4  // (S, N)        -> N: 1bit + (emis_size=5): 3bits
#define BBITS 2  // (S, N, E, J)  -> B: 2bits
#define EBITS 15 // (Mk, Dk)      -> E: 1bit + (core_size=16,384): 14bits
#define CBITS 4  // (E, C)        -> C: 1bit + (emis_size=5): 3bits
#define TBITS 1  // (E, C)        -> T: 1bit
#define JBITS 4  // (E, J)        -> J: 1bit + (emis_size=5): 3bits

// Total number of bits used by special states
#define SPECIAL_BITS (SBITS + NBITS + BBITS + EBITS + CBITS + TBITS + JBITS)
static_assert(SPECIAL_BITS <= sizeof(uint32_t) * CHAR_BIT, "");

// Number of bits used by each core state
#define MBITS 5 // (B, Mk, Ik, Dk) -> Mn: 2bits + (emis_size=5): 3bits
#define DBITS 1 // (Mk, Dk)        -> Dn: 1bit
#define IBITS 4 // (Mk, Ik)        -> Ik: 1bits + (emis_size=5): 3bits

// Total number of bits used by core states
#define CORE_BITS (MBITS + DBITS + IBITS)
static_assert(CORE_BITS <= sizeof(uint16_t) * CHAR_BIT, "");

struct trellis
{
  int core_size;

  uint32_t *xnodes;
  uint16_t *nodes;

  uint32_t *xnode;
  uint16_t *node;
};

void trellis_init(struct trellis *);
int trellis_setup(struct trellis *, int core_size, int seq_size);
void trellis_cleanup(struct trellis *);
unsigned trellis_get_previous_state(struct trellis const *, unsigned id);
unsigned trellis_get_emission_size(struct trellis const *, unsigned id);

// clang-format off
DCP_CONST unsigned trellis_xnode_get_field(uint32_t x, int state)
{
  if (state == STATE_S) return bit_extract(x, 0                                                , SBITS);
  if (state == STATE_N) return bit_extract(x, 0 + SBITS                                        , NBITS);
  if (state == STATE_B) return bit_extract(x, 0 + SBITS + NBITS                                , BBITS);
  if (state == STATE_E) return bit_extract(x, 0 + SBITS + NBITS + BBITS                        , EBITS);
  if (state == STATE_C) return bit_extract(x, 0 + SBITS + NBITS + BBITS + EBITS                , CBITS);
  if (state == STATE_T) return bit_extract(x, 0 + SBITS + NBITS + BBITS + EBITS + CBITS        , TBITS);
  if (state == STATE_J) return bit_extract(x, 0 + SBITS + NBITS + BBITS + EBITS + CBITS + TBITS, JBITS);
  DCP_UNREACHABLE();
  return 0;
}

DCP_CONST int trellis_node_get_field(uint16_t x, int state)
{
  if (dcp_state_is_match(state))  return bit_extract(x, 0                , MBITS);
  if (dcp_state_is_delete(state)) return bit_extract(x, 0 + MBITS        , DBITS);
  if (dcp_state_is_insert(state)) return bit_extract(x, 0 + MBITS + DBITS, IBITS);
  DCP_UNREACHABLE();
  return 0;
}
// clang-format on

DCP_INLINE void trellis_next_xnode(struct trellis *x) { x->xnode++; }
DCP_INLINE void trellis_next_node(struct trellis *x) { x->node++; }

DCP_INLINE void trellis_seek_xnode(struct trellis *x, int stage)
{
  x->xnode = x->xnodes + stage;
}

DCP_INLINE void trellis_seek_node(struct trellis *x, int stage, int core_idx)
{
  x->node = x->nodes + stage * x->core_size + core_idx;
}

DCP_INLINE void trellis_clear_xnode(struct trellis *x) { *x->xnode = 0; }
DCP_INLINE void trellis_clear_node(struct trellis *x) { *x->node = 0; }

// clang-format off
DCP_INLINE void trellis_set(struct trellis *x, unsigned id, int value)
{
  if (id == STATE_S) *x->xnode |= value << (0);
  if (id == STATE_N) *x->xnode |= value << (0 + SBITS);
  if (id == STATE_B) *x->xnode |= value << (0 + SBITS + NBITS);
  if (id == STATE_E) *x->xnode |= value << (0 + SBITS + NBITS + BBITS);
  if (id == STATE_C) *x->xnode |= value << (0 + SBITS + NBITS + BBITS + EBITS);
  if (id == STATE_T) *x->xnode |= value << (0 + SBITS + NBITS + BBITS + EBITS + CBITS);
  if (id == STATE_J) *x->xnode |= value << (0 + SBITS + NBITS + BBITS + EBITS + CBITS + TBITS);
  if (dcp_state_is_match(id))  *x->node |= value << (0);
  if (dcp_state_is_delete(id)) *x->node |= value << (0 + MBITS);
  if (dcp_state_is_insert(id)) *x->node |= value << (0 + MBITS + DBITS);
}

DCP_INLINE void trellis_replace(struct trellis *x, unsigned id, int value)
{
  assert(state == STATE_B);
  *x->xnode &= ~bit_rangeset(SBITS + NBITS, BBITS);
  trellis_set(x, id, value);
}
// clang-format on

#endif
