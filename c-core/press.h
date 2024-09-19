#ifndef PRESS_H
#define PRESS_H

#include <stdbool.h>

struct press;

struct press *press_new(void);
int           press_setup(struct press *, int gencode_id, float epsilon);
int           press_open(struct press *, char const *hmm, char const *db);
long          press_nproteins(struct press const *);
int           press_next(struct press *);
bool          press_end(struct press const *);
int           press_close(struct press *);
void          press_del(struct press const *);

#endif
