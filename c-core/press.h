#ifndef PRESS_H
#define PRESS_H

#include "api.h"
#include <stdbool.h>

struct press;

// clang-format off
API struct press *press_new(void);
API int           press_setup(struct press *, int gencode_id, float epsilon);
API int           press_open(struct press *, char const *hmm, char const *db);
API long          press_nproteins(struct press const *);
API int           press_next(struct press *);
API bool          press_end(struct press const *);
API int           press_close(struct press *);
API void          press_del(struct press const *);
// clang-format on

#endif
