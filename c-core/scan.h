#ifndef SCAN_H
#define SCAN_H

#include "compiler.h"
#include "scan_params.h"
#include "sequence.h"

struct scan;

DCP_API struct scan *scan_new(void);
DCP_API int scan_dial(struct scan *, int port);
DCP_API int scan_setup(struct scan *, struct scan_params);
DCP_API void scan_del(struct scan const *);

DCP_API int scan_open(struct scan *, char const *dbfile);
DCP_API int scan_add(struct scan *, long id, char const *name,
                     char const *data);
DCP_API int scan_run(struct scan *, char const *product_dir);
DCP_API int scan_close(struct scan *);

#endif
