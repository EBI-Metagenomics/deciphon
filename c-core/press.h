#ifndef PRESS_H
#define PRESS_H

#include "compiler.h"
#include <stdbool.h>

struct press;

DCP_API struct press *press_new(void);
DCP_API int press_setup(struct press *, int gencode_id, float epsilon);
DCP_API int press_open(struct press *, char const *hmm, char const *db);
DCP_API long press_nproteins(struct press const *);
DCP_API int press_next(struct press *);
DCP_API bool press_end(struct press const *);
DCP_API int press_close(struct press *);
DCP_API void press_del(struct press const *);

#endif
