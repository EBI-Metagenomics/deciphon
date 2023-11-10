#include "trellis.h"
#include "error.h"
#include "rc.h"
#include "xrealloc.h"
#include <stdlib.h>

CONST unsigned xnode_get_field(uint32_t x, int state);
CONST unsigned node_get_field(uint16_t x, int state);

void trellis_init(struct trellis *x)
{
  x->core_size = 0;
  x->xnodes = NULL;
  x->nodes = NULL;
  x->xnode = NULL;
  x->node = NULL;
}

int trellis_setup(struct trellis *x, int core_size, int seq_size)
{
  x->core_size = core_size;

  size_t num_stages = seq_size + 1;
  x->xnodes = xrealloc(x->xnodes, sizeof(*x->xnodes) * num_stages);
  x->nodes = xrealloc(x->nodes, sizeof(*x->nodes) * (num_stages * core_size));

  xstatic_assert(CHAR_BIT * sizeof(*x->xnodes) >= SPECIAL_BITS);
  xstatic_assert(CHAR_BIT * sizeof(*x->nodes) >= CORE_BITS);

  if (!x->xnodes || !x->nodes)
  {
    free(x->xnodes);
    free(x->nodes);
    x->xnodes = NULL;
    x->nodes = NULL;
    return error(DCP_ENOMEM);
  }

  return 0;
}

void trellis_cleanup(struct trellis *x)
{
  x->core_size = 0;
  free(x->xnodes);
  free(x->nodes);
  x->xnode = x->xnodes = NULL;
  x->node = x->nodes = NULL;
}

int trellis_previous_state(struct trellis const *x, int id)
{
  if (!state_is_core(id))
  {
    unsigned v = xnode_get_field(*x->xnode, id);
    if (id == STATE_S) return (int[]){STATE_S}[v];
    if (id == STATE_N) return (int[]){STATE_S, STATE_N}[v / 5];
    if (id == STATE_B) return (int[]){STATE_S, STATE_N, STATE_E, STATE_J}[v];
    if (id == STATE_E && v % 2 == 0) return state_make_match_id(v / 2);
    if (id == STATE_E && v % 2 == 1) return state_make_delete_id(v / 2);
    if (id == STATE_C) return (int[]){STATE_E, STATE_C}[v / 5];
    if (id == STATE_T) return (int[]){STATE_E, STATE_C}[v];
    if (id == STATE_J) return (int[]){STATE_E, STATE_J}[v / 5];
  }

  int idx = (x->node - x->nodes) % x->core_size;
  unsigned v = node_get_field(*x->node, id);

  if (state_is_match(id))
  {
    int s = (int[]){STATE_B, STATE_M, STATE_I, STATE_D}[v / 5];
    if (s == STATE_B) return STATE_B;
    assert(idx > 0);
    if (s == STATE_M) return state_make_match_id(idx - 1);
    if (s == STATE_I) return state_make_insert_id(idx - 1);
    if (s == STATE_D) return state_make_delete_id(idx - 1);
    UNREACHABLE();
  }

  if (state_is_delete(id))
  {
    int s = (int[]){STATE_M, STATE_D}[v];
    assert(idx > 0);
    if (s == STATE_M) return state_make_match_id(idx - 1);
    if (s == STATE_D) return state_make_delete_id(idx - 1);
    UNREACHABLE();
  }

  if (state_is_insert(id))
  {
    int s = (int[]){STATE_M, STATE_I}[v / 5];
    if (s == STATE_M) return state_make_match_id(idx);
    if (s == STATE_I) return state_make_insert_id(idx);
    UNREACHABLE();
  }

  UNREACHABLE();
  return 0;
}

int trellis_emission_size(struct trellis const *x, int id)
{
  if (id == STATE_S) return 0;
  if (id == STATE_N) return xnode_get_field(*x->xnode, STATE_N) % 5 + 1;
  if (id == STATE_B) return 0;
  if (id == STATE_E) return 0;
  if (id == STATE_C) return xnode_get_field(*x->xnode, STATE_C) % 5 + 1;
  if (id == STATE_T) return 0;
  if (id == STATE_J) return xnode_get_field(*x->xnode, STATE_J) % 5 + 1;

  if (state_is_delete(id)) return 0;
  return node_get_field(*x->node, id) % 5 + 1;
}

void trellis_seek_xnode(struct trellis *x, int stage)
{
  x->xnode = x->xnodes + stage;
}

void trellis_seek_node(struct trellis *x, int stage, int core_idx)
{
  x->node = x->nodes + stage * x->core_size + core_idx;
}

// clang-format off
CONST unsigned xnode_get_field(uint32_t x, int state)
{
  if (state == STATE_S) return bit_extract(x, 0                                                , SBITS);
  if (state == STATE_N) return bit_extract(x, 0 + SBITS                                        , NBITS);
  if (state == STATE_B) return bit_extract(x, 0 + SBITS + NBITS                                , BBITS);
  if (state == STATE_E) return bit_extract(x, 0 + SBITS + NBITS + BBITS                        , EBITS);
  if (state == STATE_C) return bit_extract(x, 0 + SBITS + NBITS + BBITS + EBITS                , CBITS);
  if (state == STATE_T) return bit_extract(x, 0 + SBITS + NBITS + BBITS + EBITS + CBITS        , TBITS);
  if (state == STATE_J) return bit_extract(x, 0 + SBITS + NBITS + BBITS + EBITS + CBITS + TBITS, JBITS);
  UNREACHABLE();
  return 0;
}

CONST unsigned node_get_field(uint16_t x, int state)
{
  if (state_is_match(state))  return bit_extract(x, 0                , MBITS);
  if (state_is_delete(state)) return bit_extract(x, 0 + MBITS        , DBITS);
  if (state_is_insert(state)) return bit_extract(x, 0 + MBITS + DBITS, IBITS);
  UNREACHABLE();
  return 0;
}
// clang-format on
