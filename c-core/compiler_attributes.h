#ifndef COMPILER_ATTRIBUTES_H
#define COMPILER_ATTRIBUTES_H

#define always_inline inline __attribute__((__always_inline__))

#define attribute_noreturn __attribute__((__noreturn__))
#define attribute_const    __attribute__((__const__))
#define attribute_pure     __attribute__((__pure__))
#define attribute_packed   __attribute__((packed))

#endif
