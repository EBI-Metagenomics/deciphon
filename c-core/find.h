#ifndef FIND_H
#define FIND_H

#if __ARM_NEON
#include "find_neon.h"
#endif

#if __AVX__
#include "find_avx.h"
#endif

#include "find_generic.h"

#endif
