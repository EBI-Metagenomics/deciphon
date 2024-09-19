#ifndef COMPILER_ATTRIBUTES_H
#define COMPILER_ATTRIBUTES_H

#define ALWAYS_INLINE      inline __attribute__((__always_inline__))
#define ATTRIBUTE_NORETURN __attribute__((__noreturn__))
#define ATTRIBUTE_CONST    __attribute__((__const__))
#define ATTRIBUTE_PURE     __attribute__((__pure__))
#define ATTRIBUTE_PACKED   __attribute__((packed))

#endif
