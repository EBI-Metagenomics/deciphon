#ifndef UNPACK_H
#define UNPACK_H

struct imm_abc;
struct lip_file;

int unpack_mapsize(struct lip_file *, unsigned size);
int unpack_key(struct lip_file *, char const key[]);
int unpack_abc(struct lip_file *, struct imm_abc *);
int unpack_f32array(struct lip_file *, unsigned size, float array[]);
int read_str(struct lip_file *, unsigned size, char str[]);

#define unpack_int(file, v)   (lip_read_int(file, v)   ? 0 : error(DCP_EFREAD))
#define unpack_float(file, v) (lip_read_float(file, v) ? 0 : error(DCP_EFREAD))
#define unpack_bool(file, v)  (lip_read_bool(file, v)  ? 0 : error(DCP_EFREAD))

#endif
