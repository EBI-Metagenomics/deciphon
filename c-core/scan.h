#ifndef SCAN_H
#define SCAN_H

#include "compiler.h"
#include "scan_params.h"

struct scan;

// clang-format off
DCP_API struct scan *scan_new(struct scan_params);
DCP_API void         scan_del(struct scan const *);
DCP_API int          scan_dial(struct scan *, int port);
DCP_API int          scan_open(struct scan *, char const *dbfile);
DCP_API void         scan_close(struct scan *);
DCP_API int          scan_add(struct scan *, long id, char const *name,
                              char const *data);
DCP_API int          scan_run(struct scan *, char const *product_dir);
// clang-format on

#endif
