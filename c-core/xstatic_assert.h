#ifndef XSTATIC_ASSERT_H
#define XSTATIC_ASSERT_H

#define XSTATIC_ASSERT_XSTR(x) XSTATIC_ASSERT_STR(x)
#define XSTATIC_ASSERT_STR(x) #x

#ifdef static_assert
#define xstatic_assert(expr) static_assert(expr, XSTATIC_ASSERT_XSTR(expr))
#else
#define xstatic_assert(expr) _Static_assert(expr, XSTATIC_ASSERT_XSTR(expr))
#endif

#endif
