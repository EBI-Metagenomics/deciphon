#ifndef TRELLIS_BITS_H
#define TRELLIS_BITS_H

#include "xstatic_assert.h"
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
xstatic_assert(SPECIAL_BITS <= sizeof(uint32_t) * CHAR_BIT);

// Number of bits used by each core state
#define MBITS 5 // (B, Mk, Ik, Dk) -> Mn: 2bits + (emis_size=5): 3bits
#define DBITS 1 // (Mk, Dk)        -> Dn: 1bit
#define IBITS 4 // (Mk, Ik)        -> Ik: 1bits + (emis_size=5): 3bits

// Total number of bits used by core states
#define CORE_BITS (MBITS + DBITS + IBITS)
xstatic_assert(CORE_BITS <= sizeof(uint16_t) * CHAR_BIT);

enum
{
  PAST_SIZE = 6
};

#endif
