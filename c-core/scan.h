#ifndef SCAN_H
#define SCAN_H

#include "compiler.h"
#include "scan_params.h"

struct scan;

// clang-format off
API struct scan *scan_new(struct scan_params);
API void         scan_del(struct scan const *);
API int          scan_dial(struct scan *, int port);
API int          scan_open(struct scan *, char const *dbfile);
API int          scan_close(struct scan *);
API int          scan_add(struct scan *, long id, char const *name,
                          char const *data);
API int          scan_run(struct scan *, char const *product_dir);
// clang-format on

#endif
