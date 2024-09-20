#ifndef EXPECT_H
#define EXPECT_H

#include <stdint.h>

struct lio_reader;

int expect_key(struct lio_reader *, char const *key);
int expect_map(struct lio_reader *, uint32_t size);

#endif
