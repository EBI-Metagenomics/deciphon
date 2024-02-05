#ifndef TRELLIS_H
#define TRELLIS_H

#include "bit.h"
#include "compiler.h"
#include "state.h"
#include "trellis_bits.h"

struct imm_path;

struct trellis
{
  int core_size;

  uint32_t *xnodes;
  uint16_t *nodes;

  uint32_t *xnode;
  uint16_t *node;
};

// clang-format off
void        trellis_init(struct trellis *);
int         trellis_setup(struct trellis *, int core_size, int seq_size);
void        trellis_cleanup(struct trellis *);
int         trellis_previous_state(struct trellis const *, int id);
int         trellis_emission_size(struct trellis const *, int id);
INLINE void trellis_next_xnode(struct trellis *x) { x->xnode++; }
INLINE void trellis_next_node(struct trellis *x) { x->node++; }
void        trellis_seek_xnode(struct trellis *x, int stage);
void        trellis_seek_node(struct trellis *x, int stage, int core_idx);
INLINE void trellis_clear_xnode(struct trellis *x) { *x->xnode = 0; }
INLINE void trellis_clear_node(struct trellis *x) { *x->node = 0; }
int         trellis_unzip(struct trellis *, int seq_size, struct imm_path *);
// clang-format on

// clang-format off
INLINE void trellis_set(struct trellis *x, int id, int value)
{
  /* long sz = x->node - x->nodes; */
  /* long stage = sz / x->core_size; */
  /* long k = sz - stage * x->core_size; */
  /* if      (id == STATE_S) printf("S: %d\n", value); */
  /* else if (id == STATE_N) printf("N: %d\n", value); */
  /* else if (id == STATE_B) printf("B: %d\n", value); */
  /* else if (id == STATE_E) printf("E: %d\n", value); */
  /* else if (id == STATE_C) printf("C: %d\n", value); */
  /* else if (id == STATE_T) printf("T: %d\n", value); */
  /* else if (id == STATE_J) printf("J: %d\n", value); */
  /* else if (state_is_match(id))  printf("M%ld: %d\n", k, value); */
  /* else if (state_is_delete(id)) printf("D%ld: %d\n", k, value); */
  /* else if (state_is_insert(id)) printf("I%ld: %d\n", k, value); */
  /* else UNREACHABLE(); */

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

INLINE void trellis_replace(struct trellis *x, int id, int value)
{
  assert(id == STATE_B);
  *x->xnode &= ~bit_rangeset(SBITS + NBITS, BBITS);
  trellis_set(x, id, value);
}
// clang-format on

#endif
