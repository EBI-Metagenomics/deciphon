#ifndef TRELLIS_H
#define TRELLIS_H

#include "compiler.h"
#include "state.h"
#include "build_bug.h"
#include <limits.h>
#include <stdint.h>

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
static_assert(SPECIAL_BITS <= sizeof(uint32_t) * CHAR_BIT);

// Number of bits used by each core state
#define MBITS 5 // (B, Mk, Ik, Dk) -> Mn: 2bits + (emis_size=5): 3bits
#define DBITS 1 // (Mk, Dk)        -> Dn: 1bit
#define IBITS 4 // (Mk, Ik)        -> Ik: 1bits + (emis_size=5): 3bits

// Total number of bits used by core states
#define CORE_BITS (MBITS + DBITS + IBITS)
static_assert(CORE_BITS <= sizeof(uint16_t) * CHAR_BIT);

struct imm_path;

struct trellis
{
  int core_size;

  uint32_t *xnodes;
  uint16_t *nodes;

  uint32_t *xnode;
  uint16_t *node;
};

void        trellis_init(struct trellis *);
int         trellis_setup(struct trellis *, int core_size, int seq_size);
void        trellis_cleanup(struct trellis *);
INLINE void trellis_next_xnode(struct trellis *x) { x->xnode++; }
INLINE void trellis_next_node(struct trellis *x) { x->node++; }
void        trellis_seek_xnode(struct trellis *x, int stage);
void        trellis_seek_node(struct trellis *x, int stage, int core_idx);
INLINE void trellis_clear_xnode(struct trellis *x) { *x->xnode = 0; }
INLINE void trellis_clear_node(struct trellis *x) { *x->node = 0; }
int         trellis_unzip(struct trellis *, int seq_size, struct imm_path *);

INLINE void trellis_set(struct trellis *x, int id, int value)
{
  unsigned v = *(unsigned *)&value;
  if      (id == STATE_S) *x->xnode |= v << (0);
  else if (id == STATE_N) *x->xnode |= v << (0 + SBITS);
  else if (id == STATE_B) *x->xnode |= v << (0 + SBITS + NBITS);
  else if (id == STATE_E) *x->xnode |= v << (0 + SBITS + NBITS + BBITS);
  else if (id == STATE_C) *x->xnode |= v << (0 + SBITS + NBITS + BBITS + EBITS);
  else if (id == STATE_T) *x->xnode |= v << (0 + SBITS + NBITS + BBITS + EBITS + CBITS);
  else if (id == STATE_J) *x->xnode |= v << (0 + SBITS + NBITS + BBITS + EBITS + CBITS + TBITS);
  else if (state_is_match(id))  *x->node |= v << (0);
  else if (state_is_delete(id)) *x->node |= v << (0 + MBITS);
  else if (state_is_insert(id)) *x->node |= v << (0 + MBITS + DBITS);
  else UNREACHABLE();
}

#endif
