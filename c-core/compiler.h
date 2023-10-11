#ifndef DECIPHON_COMPILER_H
#define DECIPHON_COMPILER_H

#ifdef __has_builtin
#define DCP_HAS_BUILTIN(x) __has_builtin(x)
#else
#define DCP_HAS_BUILTIN(x) (0)
#endif

#if DCP_HAS_BUILTIN(__builtin_unreachable)
#define DCP_UNREACHABLE() __builtin_unreachable()
#else
#define DCP_UNREACHABLE() (void)(0)
#endif

#ifdef __has_attribute
#define DCP_HAS_ATTRIBUTE(x) __has_attribute(x)
#else
#define DCP_HAS_ATTRIBUTE(x) (0)
#endif

#if DCP_HAS_ATTRIBUTE(unused)
#define DCP_UNUSED __attribute__((unused))
#else
#define DCP_UNUSED
#endif

#if DCP_HAS_BUILTIN(__builtin_prefetch)
#define DCP_PREFETCH(addr, rw, locality) __builtin_prefetch(addr, rw, locality)
#else
#define DCP_PREFETCH(addr, rw, locality) (void)(0)
#endif

#if DCP_HAS_ATTRIBUTE(always_inline)
#define DCP_INLINE static inline __attribute__((always_inline))
#else
#define DCP_INLINE static inline
#endif

#if DCP_HAS_ATTRIBUTE(const)
#define DCP_CONST DCP_INLINE __attribute__((const))
#else
#define DCP_CONST DCP_INLINE
#endif

#if DCP_HAS_ATTRIBUTE(pure)
#define DCP_PURE DCP_INLINE __attribute__((pure))
#else
#define DCP_PURE DCP_INLINE
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
