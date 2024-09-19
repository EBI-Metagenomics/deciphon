#ifndef COMPILER_H
#define COMPILER_H

#ifdef __has_builtin
#define HAS_BUILTIN(x) __has_builtin(x)
#else
#define HAS_BUILTIN(x) (0)
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

#if HAS_BUILTIN(__builtin_unreachable)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define unlikely(x) (x)
#endif

#if HAS_BUILTIN(__builtin_unreachable)
#define unreachable() __builtin_unreachable()
#else
#define unreachable() (void)(0)
#endif

#if HAS_ATTRIBUTE(__always_inline__)
#define ALWAYS_INLINE inline __attribute__((__always_inline__))
#else
#define ALWAYS_INLINE inline
#endif

#if HAS_ATTRIBUTE(__noreturn__)
#define ATTRIBUTE_NORETURN __attribute__((__noreturn__))
#else
#define ATTRIBUTE_NORETURN
#endif

#if HAS_ATTRIBUTE(__const__)
#define ATTRIBUTE_CONST __attribute__((__const__))
#else
#define ATTRIBUTE_CONST
#endif

#if HAS_ATTRIBUTE(__pure__)
#define ATTRIBUTE_PURE __attribute__((__pure__))
#else
#define ATTRIBUTE_PURE
#endif

#if HAS_ATTRIBUTE(packed)
#define ATTRIBUTE_PACKED __attribute__((packed))
#else
#define ATTRIBUTE_PACKED
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

#endif
