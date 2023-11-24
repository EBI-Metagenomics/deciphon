#ifndef VMAX_H
#define VMAX_H

#if __ARM_NEON
#include "vmax_neon.h"
#endif

#if __AVX__
#include "vmax_avx.h"
#endif

#include "vmax_generic.h"

#endif
