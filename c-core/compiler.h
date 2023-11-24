#ifndef COMPILER_H
#define COMPILER_H

#include <assert.h>

#ifdef __has_builtin
#define HAS_BUILTIN(x) __has_builtin(x)
#else
#define HAS_BUILTIN(x) (0)
#endif

#if HAS_BUILTIN(__builtin_unreachable)
#define UNREACHABLE() __builtin_unreachable()
#else
#define UNREACHABLE() (void)(0)
#endif

#ifdef __has_attribute
#define HAS_ATTRIBUTE(x) __has_attribute(x)
#else
#define HAS_ATTRIBUTE(x) (0)
#endif

#if HAS_ATTRIBUTE(unused)
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

#if HAS_BUILTIN(__builtin_prefetch)
#define PREFETCH(addr, rw, locality) __builtin_prefetch(addr, rw, locality)
#else
#define PREFETCH(addr, rw, locality)                                           \
  do                                                                           \
  {                                                                            \
    (void)(addr);                                                              \
    (void)(rw);                                                                \
    (void)(locality);                                                          \
  } while (0)
#endif

#if HAS_ATTRIBUTE(always_inline)
#define INLINE static inline __attribute__((always_inline))
#else
#define INLINE static inline
#endif

#if HAS_ATTRIBUTE(const)
#define CONST INLINE __attribute__((const))
#else
#define CONST INLINE
#endif

#if HAS_ATTRIBUTE(pure)
#define PURE INLINE __attribute__((pure))
#else
#define PURE INLINE
#endif

#if HAS_ATTRIBUTE(format)
#define FORMAT(a, b) __attribute__((format(printf, a, b)))
#else
#define FORMAT (a, b)
#endif

#if !__AVX__ && !__ARM_NEON
#error "Needs either AVX or NEON CPU extension"
#endif

#if __AVX__
#define ALIGNED __attribute__((aligned(32)))
#define ASSUME_ALIGNED(x) __builtin_assume_aligned(x, 32)
#endif

#if __ARM_NEON
#define ALIGNED __attribute__((aligned(16)))
#define ASSUME_ALIGNED(x) __builtin_assume_aligned(x, 16)
#endif

UNUSED CONST int bug_on_reach(void)
{
  assert(0);
  return 0;
}

#endif
