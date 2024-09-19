#ifndef TRELLIS_H
#define TRELLIS_H

#include "build_bug.h"
#include "compiler_attributes.h"
#include "state.h"
#include <limits.h>
#include <stdint.h>

struct imm_path;

struct trellis
{
  int core_size;

  uint32_t *xnodes;
  uint16_t *nodes;

  uint32_t *xnode;
  uint16_t *node;
};

static_assert((STATE_S_BITS + STATE_N_BITS + STATE_B_BITS + STATE_E_BITS +
               STATE_C_BITS + STATE_T_BITS + STATE_J_BITS) <=
              sizeof(uint32_t) * CHAR_BIT);

static_assert((STATE_M_BITS + STATE_D_BITS + STATE_I_BITS) <=
              sizeof(uint16_t) * CHAR_BIT);

void trellis_init(struct trellis *);
int  trellis_setup(struct trellis *, int core_size, int seq_size);
void trellis_cleanup(struct trellis *);
void trellis_seek_xnode(struct trellis *x, int stage);
void trellis_seek_node(struct trellis *x, int stage, int core_idx);
int  trellis_unzip(struct trellis *, int seq_size, struct imm_path *);

static inline void trellis_next_xnode(struct trellis *x) { x->xnode++; }
static inline void trellis_next_node(struct trellis *x) { x->node++; }
static inline void trellis_clear_xnode(struct trellis *x) { *x->xnode = 0; }
static inline void trellis_clear_node(struct trellis *x) { *x->node = 0; }

static always_inline void trellis_set(struct trellis *x, int id, int value)
{

  unsigned v = *(unsigned *)&value;
  if (id == STATE_S) *x->xnode |= v << (0);
  if (id == STATE_N) *x->xnode |= v << (0 + STATE_S_BITS);
  if (id == STATE_B) *x->xnode |= v << (0 + STATE_S_BITS + STATE_N_BITS);
  if (id == STATE_E) *x->xnode |= v << (0 + STATE_S_BITS + STATE_N_BITS + STATE_B_BITS);
  if (id == STATE_C) *x->xnode |= v << (0 + STATE_S_BITS + STATE_N_BITS + STATE_B_BITS + STATE_E_BITS);
  if (id == STATE_T) *x->xnode |= v << (0 + STATE_S_BITS + STATE_N_BITS + STATE_B_BITS + STATE_E_BITS + STATE_C_BITS);
  if (id == STATE_J) *x->xnode |= v << (0 + STATE_S_BITS + STATE_N_BITS + STATE_B_BITS + STATE_E_BITS + STATE_C_BITS + STATE_T_BITS);
  if (state_is_match(id))  *x->node |= v << (0);
  if (state_is_delete(id)) *x->node |= v << (0 + STATE_M_BITS);
  if (state_is_insert(id)) *x->node |= v << (0 + STATE_M_BITS + STATE_D_BITS);
}

#endif
