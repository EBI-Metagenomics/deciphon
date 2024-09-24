#include "trellis.h"
#include "bit.h"
#include "bug.h"
#include "deciphon.h"
#include "error.h"
#include "imm_path.h"
#include "xrealloc.h"
#include <stdlib.h>

ATTRIBUTE_CONST unsigned xnode_get_field(uint32_t x, int state);
ATTRIBUTE_CONST unsigned node_get_field(uint16_t x, int state);

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

static int previous_state(struct trellis const *x, int id)
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
    BUG_ON(idx <= 0);
    if (s == STATE_M) return state_make_match_id(idx - 1);
    if (s == STATE_I) return state_make_insert_id(idx - 1);
    if (s == STATE_D) return state_make_delete_id(idx - 1);
    unreachable();
  }

  if (state_is_delete(id))
  {
    int s = (int[]){STATE_M, STATE_D}[v];
    BUG_ON(idx <= 0);
    if (s == STATE_M) return state_make_match_id(idx - 1);
    if (s == STATE_D) return state_make_delete_id(idx - 1);
    unreachable();
  }

  if (state_is_insert(id))
  {
    int s = (int[]){STATE_M, STATE_I}[v / 5];
    if (s == STATE_M) return state_make_match_id(idx);
    if (s == STATE_I) return state_make_insert_id(idx);
    unreachable();
  }

  unreachable();
  return 0;
}

static int emission_size(struct trellis const *x, int id)
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

ATTRIBUTE_CONST unsigned xnode_get_field(uint32_t x, int state)
{
  if (state == STATE_S) return bit_extract(x, 0, STATE_S_BITS);
  if (state == STATE_N) return bit_extract(x, 0 + STATE_S_BITS, STATE_N_BITS);
  if (state == STATE_B) return bit_extract(x, 0 + STATE_S_BITS + STATE_N_BITS, STATE_B_BITS);
  if (state == STATE_E) return bit_extract(x, 0 + STATE_S_BITS + STATE_N_BITS + STATE_B_BITS, STATE_E_BITS);
  if (state == STATE_C) return bit_extract(x, 0 + STATE_S_BITS + STATE_N_BITS + STATE_B_BITS + STATE_E_BITS, STATE_C_BITS);
  if (state == STATE_T) return bit_extract(x, 0 + STATE_S_BITS + STATE_N_BITS + STATE_B_BITS + STATE_E_BITS + STATE_C_BITS, STATE_T_BITS);
  if (state == STATE_J) return bit_extract(x, 0 + STATE_S_BITS + STATE_N_BITS + STATE_B_BITS + STATE_E_BITS + STATE_C_BITS + STATE_T_BITS, STATE_J_BITS);
  unreachable();
  return 0;
}

ATTRIBUTE_CONST unsigned node_get_field(uint16_t x, int state)
{
  if (state_is_match(state))  return bit_extract(x, 0, STATE_M_BITS);
  if (state_is_delete(state)) return bit_extract(x, 0 + STATE_M_BITS, STATE_D_BITS);
  if (state_is_insert(state)) return bit_extract(x, 0 + STATE_M_BITS + STATE_D_BITS, STATE_I_BITS);
  unreachable();
  return 0;
}

int trellis_unzip(struct trellis *x, int L, struct imm_path *path)
{
  int state = state_make_end();
  int stage = L;
  trellis_seek_xnode(x, stage);

  while (!state_is_start(state) || stage)
  {
    int size = emission_size(x, state);
    if (imm_path_add(path, imm_step(state, size, 0))) return error(DCP_ENOMEM);
    state = previous_state(x, state);
    stage -= size;
    if (state_is_core(state))
      trellis_seek_node(x, stage, state_core_idx(state));
    else
      trellis_seek_xnode(x, stage);
  }
  if (imm_path_add(path, imm_step(state, 0, 0))) return error(DCP_ENOMEM);
  imm_path_reverse(path);
  return 0;
}
