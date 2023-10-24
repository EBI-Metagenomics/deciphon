#ifndef VITERBI_EMISSION_H
#define VITERBI_EMISSION_H

#include "compiler.h"
#include "imm/imm.h"
#include "viterbi_bits.h"
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

#define DECLARE_EMISSION_INDEX(name) int name[PAST_SIZE - 1] ALIGNED
#define DECLARE_EMISSION_TABLE(name) float name[PAST_SIZE - 1] ALIGNED

INLINE void emission_index(int index[restrict], struct imm_eseq const *eseq,
                           int row, bool const safe)
{
#pragma omp unroll
  for (int i = 0; i < PAST_SIZE - 1; ++i)
    index[i] = get_index(eseq, row - i - 1, i + 1, safe);
}

INLINE void emission_fetch(float x[restrict], float const emission[restrict],
                           int const index[restrict], bool const safe)
{
#pragma omp unroll
  for (int i = 0; i < PAST_SIZE - 1; ++i)
    x[i] = get_emission(emission, index[i], safe);
}

INLINE void emission_prefetch(float emission[restrict],
                              int const index[restrict])
{
#pragma omp unroll
  for (int i = 0; i < PAST_SIZE - 1; ++i)
    PREFETCH(get_emission_addr(emission, index[i]), 0, 1);
}

CONST float emission_of(float const emission[restrict], int num_chars)
{
  float const *e = ASSUME_ALIGNED(emission);
  return e[num_chars - 1];
}

#endif
