#ifndef ARGMAX_H
#define ARGMAX_H

#if __ARM_NEON
#include "argmax_neon.h"
#endif

#if __AVX__
#include "argmax_avx.h"
#endif

#include "argmax_generic.h"

#endif
