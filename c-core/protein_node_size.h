#ifndef DECIPHON_PROTEIN_NODE_SIZE_H
#define DECIPHON_PROTEIN_NODE_SIZE_H

#define PROTEIN_NODE_SIZE                                                      \
  (IMM_NUCLT_SIZE + IMM_NUCLT_SIZE * IMM_NUCLT_SIZE +                          \
   IMM_NUCLT_SIZE * IMM_NUCLT_SIZE * IMM_NUCLT_SIZE +                          \
   IMM_NUCLT_SIZE * IMM_NUCLT_SIZE * IMM_NUCLT_SIZE * IMM_NUCLT_SIZE +         \
   IMM_NUCLT_SIZE * IMM_NUCLT_SIZE * IMM_NUCLT_SIZE * IMM_NUCLT_SIZE *         \
       IMM_NUCLT_SIZE)

#endif
