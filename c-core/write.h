#ifndef WRITE_H
#define WRITE_H

#include <stdbool.h>
#include <stdint.h>

struct lio_writer;

int write_cstring(struct lio_writer *, char const *str);
int write_map(struct lio_writer *, int size);
int write_array(struct lio_writer *, int size);
int write_i(struct lio_writer *, int data);
int write_u(struct lio_writer *, unsigned data);
int write_float(struct lio_writer *, float data);
int write_bool(struct lio_writer *, bool data);
int write_f32array(struct lio_writer *, uint32_t size, float const *);

#define write_int(buffer, data)                                                \
  _Generic((data), int: write_i, unsigned: write_u)(buffer, data)

#endif
