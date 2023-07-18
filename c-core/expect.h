#ifndef DECIPHON_EXPECT_H
#define DECIPHON_EXPECT_H

struct lip_file;

int dcp_expect_map_size(struct lip_file *, unsigned size);
int dcp_expect_map_key(struct lip_file *, char const key[]);

#endif
