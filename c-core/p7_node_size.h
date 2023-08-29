#ifndef DECIPHON_P7_NODE_SIZE_H
#define DECIPHON_P7_NODE_SIZE_H

#define P7_NODE_SIZE                                                           \
  (IMM_NUCLT_SIZE + IMM_NUCLT_SIZE * IMM_NUCLT_SIZE +                          \
   IMM_NUCLT_SIZE * IMM_NUCLT_SIZE * IMM_NUCLT_SIZE +                          \
   IMM_NUCLT_SIZE * IMM_NUCLT_SIZE * IMM_NUCLT_SIZE * IMM_NUCLT_SIZE +         \
   IMM_NUCLT_SIZE * IMM_NUCLT_SIZE * IMM_NUCLT_SIZE * IMM_NUCLT_SIZE *         \
       IMM_NUCLT_SIZE)

#endif
