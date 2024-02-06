#ifndef PACK_H
#define PACK_H

struct imm_abc;
struct lip_file;

int pack_key(struct lip_file *, char const key[]);
int pack_mapsize(struct lip_file *, unsigned size);
int pack_abc(struct lip_file *, struct imm_abc const *abc);
int pack_f32array(struct lip_file *, unsigned size, float const array[]);

#define pack_int(file, v)   (lip_write_int(file, v)   ? 0 : error(DCP_EFWRITE))
#define pack_float(file, v) (lip_write_float(file, v) ? 0 : error(DCP_EFWRITE))
#define pack_bool(file, v)  (lip_write_bool(file, v)  ? 0 : error(DCP_EFWRITE))

#endif
