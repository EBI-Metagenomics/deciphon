#ifndef DCP_STATIC_ASSERT_H
#define DCP_STATIC_ASSERT_H

#include <assert.h>

#ifdef static_assert
#define dcp_static_assert(expr, msg) static_assert(expr, msg)
#else
#define dcp_static_assert(expr, msg) _Static_assert(expr, msg)
#endif

#endif
