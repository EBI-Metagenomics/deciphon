#include "viterbi_path.h"
#include "error.h"
#include "imm/path.h"
#include "rc.h"
#include "state.h"
#include "trellis.h"
#include <assert.h>

int unzip_path(struct trellis *x, int seq_size, struct imm_path *path)
{
  int state = state_make_end();
  assert(seq_size <= INT_MAX);
  int stage = seq_size;
  trellis_seek_xnode(x, stage);

  while (!state_is_start(state) || stage)
  {
    int size = trellis_emission_size(x, state);
    if (imm_path_add(path, imm_step(state, size, 0))) return error(DCP_ENOMEM);
    state = trellis_previous_state(x, state);
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
