#ifndef UNPACK_H
#define UNPACK_H

#include <stdbool.h>
#include <stdint.h>

struct lio_reader;

int read_string(struct lio_reader *, uint32_t *);
int read_cstring(struct lio_reader *, uint32_t, char *);
int read_map(struct lio_reader *, uint32_t *);
int read_array(struct lio_reader *, uint32_t *);
int read_i32(struct lio_reader *, int32_t *);
int read_u32(struct lio_reader *, uint32_t *);
int read_bool(struct lio_reader *, bool *);
int read_float(struct lio_reader *, float *);
int read_f32array(struct lio_reader *, uint32_t size, float *);

#define read_int(buffer, data)                                                 \
  _Generic((data), uint32_t *: read_u32, int32_t *: read_i32)(buffer, data)

#endif
