#ifndef EXPECT_H
#define EXPECT_H

#include <stdint.h>

struct lio_reader;

int expect_key(struct lio_reader *x, char const *key);
int expect_map(struct lio_reader *x, uint32_t size);

#endif
