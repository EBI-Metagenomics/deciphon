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

#if __has_attribute(const)
#define dcp_const __attribute__((const))
#else
#define dcp_const
#endif

#if __has_attribute(pure)
#define dcp_pure __attribute__((pure))
#else
#define dcp_pure
#endif

#if __has_attribute(always_inline)
#define dcp_force_inline __attribute__((always_inline))
#else
#define dcp_force_inline
#endif

/*
 * dcp_template is used to define C "templates", which take constant
 * parameters. They must be inlined for the compiler to eliminate the constant
 * branches.
 *
 * Acknowledgement: ZSTD.
 */
#define dcp_template static inline dcp_force_inline

#define dcp_const_template dcp_const dcp_template
#define dcp_pure_template dcp_pure dcp_template

#define DCP_INLINE static inline dcp_force_inline
#define DCP_PURE dcp_pure DCP_INLINE
#define DCP_CONST dcp_const DCP_INLINE

#endif
