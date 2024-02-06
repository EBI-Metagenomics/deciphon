#ifndef COMPILER_ATTRIBUTES_H
#define COMPILER_ATTRIBUTES_H

#define __noreturn __attribute__((__noreturn__))
#define __always_inline inline __attribute__((__always_inline__))

#define __attribute_const __attribute__((__const__))
#define __attribute_pure __attribute__((__pure__))

#endif
