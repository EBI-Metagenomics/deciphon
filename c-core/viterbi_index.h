#ifndef VITERBI_INDEX_H
#define VITERBI_INDEX_H

#include "compiler.h"
#include "imm/eseq.h"
#include "xlimits.h"
#include <stdbool.h>

#define DECLARE_INDEX(name) int name[DCP_PAST_SIZE - 1] ALIGNED

INLINE void index_setup(int index[restrict], struct imm_eseq const *eseq,
                        int row, bool const safe)
{
#pragma GCC unroll(DCP_PAST_SIZE - 1)
  for (int i = 0; i < DCP_PAST_SIZE - 1; ++i)
  {
    int pos = row - i - 1;
    int size = i + 1;
    index[i] = (!safe && pos < 0) ? -1 : imm_eseq_get(eseq, pos, size, 1);
  }
}

#endif
