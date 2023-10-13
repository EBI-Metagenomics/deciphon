#include "trellis.h"

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
  x->xnodes = dcp_reallocf(x->xnodes, sizeof(*x->xnodes) * num_stages);

  size_t size = num_stages * core_size;
  x->nodes = dcp_reallocf(x->nodes, sizeof(*x->nodes) * size);

  dcp_static_assert(CHAR_BIT * sizeof(*x->xnodes) >= SPECIAL_BITS, "");
  dcp_static_assert(CHAR_BIT * sizeof(*x->nodes) >= CORE_BITS, "");

  if (!x->xnodes || !x->nodes)
  {
    free(x->xnodes);
    free(x->nodes);
    x->xnodes = NULL;
    x->nodes = NULL;
    return DCP_ENOMEM;
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

int trellis_get_previous_state(struct trellis const *x, int id)
{
  if (!dcp_state_is_core(id))
  {
    int v = trellis_xnode_get_field(*x->xnode, id);
    if (id == STATE_S) return (int[]){STATE_S}[v];
    if (id == STATE_N) return (int[]){STATE_S, STATE_N}[v / 5];
    if (id == STATE_B) return (int[]){STATE_S, STATE_N, STATE_E, STATE_J}[v];
    if (id == STATE_E && v % 2 == 0) return dcp_state_make_match_id(v / 2);
    if (id == STATE_E && v % 2 == 1) return dcp_state_make_delete_id(v / 2);
    if (id == STATE_C) return (int[]){STATE_E, STATE_C}[v / 5];
    if (id == STATE_T) return (int[]){STATE_E, STATE_C}[v];
    if (id == STATE_J) return (int[]){STATE_E, STATE_J}[v / 5];
  }

  int idx = (x->node - x->nodes) % x->core_size;
  int v = trellis_node_get_field(*x->node, id);

  if (dcp_state_is_match(id))
  {
    int s = (int[]){STATE_B, STATE_MATCH, STATE_INSERT, STATE_DELETE}[v / 5];
    if (s == STATE_B) return STATE_B;
    assert(idx > 0);
    if (s == STATE_MATCH) return dcp_state_make_match_id(idx - 1);
    if (s == STATE_INSERT) return dcp_state_make_insert_id(idx - 1);
    if (s == STATE_DELETE) return dcp_state_make_delete_id(idx - 1);
    DCP_UNREACHABLE();
  }

  if (dcp_state_is_delete(id))
  {
    int s = (int[]){STATE_MATCH, STATE_DELETE}[v];
    assert(idx > 0);
    if (s == STATE_MATCH) return dcp_state_make_match_id(idx - 1);
    if (s == STATE_DELETE) return dcp_state_make_delete_id(idx - 1);
    DCP_UNREACHABLE();
  }

  if (dcp_state_is_insert(id))
  {
    int s = (int[]){STATE_MATCH, STATE_INSERT}[v / 5];
    if (s == STATE_MATCH) return dcp_state_make_match_id(idx);
    if (s == STATE_INSERT) return dcp_state_make_insert_id(idx);
    DCP_UNREACHABLE();
  }

  DCP_UNREACHABLE();
  return 0;
}

int trellis_get_emission_size(struct trellis const *x, int id)
{
  if (id == STATE_S) return 0;
  if (id == STATE_N) return trellis_xnode_get_field(*x->xnode, STATE_N) % 5 + 1;
  if (id == STATE_B) return 0;
  if (id == STATE_E) return 0;
  if (id == STATE_C) return trellis_xnode_get_field(*x->xnode, STATE_C) % 5 + 1;
  if (id == STATE_T) return 0;
  if (id == STATE_J) return trellis_xnode_get_field(*x->xnode, STATE_J) % 5 + 1;

  if (dcp_state_is_delete(id)) return 0;
  return trellis_node_get_field(*x->node, id) % 5 + 1;
}
