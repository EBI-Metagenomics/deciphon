#include "array_size.h"
#include "error.h"
#include <stddef.h>

#define ix(x)                                                                  \
  (x == 'A' ? 0 :                                                              \
   x == 'C' ? 1 :                                                              \
   x == 'G' ? 2 :                                                              \
   x == 'T' ? 3 :                                                              \
   x == 'U' ? 4 :                                                              \
   0)
#define ch(x)                                                                  \
  (x == 0 ? 'A' :                                                              \
   x == 1 ? 'C' :                                                              \
   x == 2 ? 'G' :                                                              \
   x == 3 ? 'T' :                                                              \
   x == 4 ? 'U' :                                                              \
   0)


static inline int max_idx(int size, int const indices[], size_t const count[])
{
  int max_index = indices[0];
  size_t max_value = count[indices[0]];
  for (int i = 1; i < size; ++i)
  {
    if (count[indices[i]] > max_value)
    {
      max_index = indices[i];
      max_value = count[indices[i]];
    }
  }
  return max_index;
}

int disambiguate(int size, char *seq)
{
  size_t count[] = {[ix('A')] = 0,
                    [ix('C')] = 0,
                    [ix('G')] = 0,
                    [ix('T')] = 0,
                    [ix('U')] = 0};

  for (int i = 0; i < size; ++i)
  {
    if (seq[i] == 'A') count[ix('A')] += 1;
    if (seq[i] == 'C') count[ix('C')] += 1;
    if (seq[i] == 'G') count[ix('G')] += 1;
    if (seq[i] == 'T') count[ix('T')] += 1;
    if (seq[i] == 'U') count[ix('U')] += 1;
  }

  if (count[ix('T')] > 0 && count[ix('U')] > 0) return error(DCP_ENUCLTSEQTU);

  int R[] = {ix('A'), ix('G')};
  int Y[] = {ix('C'), ix('T')};
  int M[] = {ix('A'), ix('C')};
  int K[] = {ix('G'), ix('T')};
  int S[] = {ix('C'), ix('G')};
  int W[] = {ix('A'), ix('T')};
  int H[] = {ix('A'), ix('C'), ix('T')};
  int B[] = {ix('C'), ix('G'), ix('T')};
  int V[] = {ix('A'), ix('C'), ix('G')};
  int D[] = {ix('A'), ix('G'), ix('T')};
  int N[] = {ix('A'), ix('C'), ix('G'), ix('T')};
  int X[] = {ix('A'), ix('C'), ix('G'), ix('T')};

  for (int i = 0; i < size; ++i)
  {
    if (seq[i] == 'R') seq[i] = ch(max_idx(array_size(R), R, count));
    if (seq[i] == 'Y') seq[i] = ch(max_idx(array_size(Y), Y, count));
    if (seq[i] == 'M') seq[i] = ch(max_idx(array_size(M), M, count));
    if (seq[i] == 'K') seq[i] = ch(max_idx(array_size(K), K, count));
    if (seq[i] == 'S') seq[i] = ch(max_idx(array_size(S), S, count));
    if (seq[i] == 'W') seq[i] = ch(max_idx(array_size(W), W, count));
    if (seq[i] == 'H') seq[i] = ch(max_idx(array_size(H), H, count));
    if (seq[i] == 'B') seq[i] = ch(max_idx(array_size(B), B, count));
    if (seq[i] == 'V') seq[i] = ch(max_idx(array_size(V), V, count));
    if (seq[i] == 'D') seq[i] = ch(max_idx(array_size(D), D, count));
    if (seq[i] == 'N') seq[i] = ch(max_idx(array_size(N), N, count));
    if (seq[i] == 'X') seq[i] = ch(max_idx(array_size(X), X, count));
  }

  return 0;
}
