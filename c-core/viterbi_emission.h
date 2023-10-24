#ifndef VITERBI_EMISSION_H
#define VITERBI_EMISSION_H

#include "compiler.h"
#include "imm/imm.h"
#include <stdbool.h>

PURE int get_index(struct imm_eseq const *x, int pos, int size, bool const safe)
{
  return ((!safe && (pos) < 0) ? -1 : imm_eseq_get(x, pos, size, 1));
}

PURE float get_emission(float const x[restrict], int i, bool const safe)
{
  return safe ? x[i] : i >= 0 ? x[i] : IMM_LPROB_ZERO;
}

PURE float const *get_emission_addr(float const x[restrict], int i)
{
  return x + i;
}

INLINE void emission_index(int index[restrict], struct imm_eseq const *eseq,
                           int row, bool const safe)
{
  index[0] = get_index(eseq, row - 1, 1, safe);
  index[1] = get_index(eseq, row - 2, 2, safe);
  index[2] = get_index(eseq, row - 3, 3, safe);
  index[3] = get_index(eseq, row - 4, 4, safe);
  index[4] = get_index(eseq, row - 5, 5, safe);
}

INLINE void emission_fetch(float x[restrict], float emission[restrict],
                           int const index[restrict], bool const safe)
{
  x[0] = get_emission(emission, index[0], safe);
  x[1] = get_emission(emission, index[1], safe);
  x[2] = get_emission(emission, index[2], safe);
  x[3] = get_emission(emission, index[3], safe);
  x[4] = get_emission(emission, index[4], safe);
}

INLINE void emission_prefetc(float emission[restrict],
                             int const index[restrict])
{
  PREFETCH(get_emission_addr(emission, index[0]), 0, 1);
  PREFETCH(get_emission_addr(emission, index[1]), 0, 1);
  PREFETCH(get_emission_addr(emission, index[2]), 0, 1);
  PREFETCH(get_emission_addr(emission, index[3]), 0, 1);
  PREFETCH(get_emission_addr(emission, index[4]), 0, 1);
}

CONST float emission_of(float const emission[restrict], int num_chars)
{
  float const *e = ASSUME_ALIGNED(emission);
  return e[num_chars - 1];
}

#endif
