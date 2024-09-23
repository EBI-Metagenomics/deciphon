#ifndef BATCH_H
#define BATCH_H

struct imm_code;

struct batch *batch_new(void);
void          batch_del(struct batch *);
int           batch_add(struct batch *, long id, char const *name, char const *data);
void          batch_reset(struct batch *);

int         batch_encode(struct batch *, struct imm_code const *);
struct iter batch_iter(struct batch const *);

#endif
