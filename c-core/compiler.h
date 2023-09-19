#ifndef DECIPHON_COMPILER_H
#define DECIPHON_COMPILER_H

#ifndef __has_builtin
#define __has_builtin(x) (0)
#endif

#if !__has_builtin(__builtin_unreachable)
#define __builtin_unreachable() (void)(0)
#endif

#ifndef __has_attribute
#define __has_attribute(x) (0)
#endif

#if __has_attribute(always_inline)
#define DCP_INLINE static inline __attribute__((always_inline))
#else
#define DCP_INLINE static inline
#endif

#if __has_attribute(const)
#define DCP_CONST __attribute__((const)) DCP_INLINE
#else
#define DCP_CONST DCP_INLINE
#endif

#if __has_attribute(pure)
#define DCP_PURE __attribute__((pure))
#else
#define DCP_PURE DCP_INLINE
#endif

#endif
