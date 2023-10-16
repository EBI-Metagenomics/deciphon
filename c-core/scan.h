#ifndef SCAN_H
#define SCAN_H

#include "compiler.h"
#include "scan_params.h"
#include "seq.h"

struct scan;

DCP_API struct scan *scan_new(void);
DCP_API int scan_dial(struct scan *, int port);
DCP_API int scan_setup(struct scan *, struct scan_params);
DCP_API void scan_del(struct scan const *);

DCP_API int scan_run(struct scan *, char const *dbfile, seq_next_fn *callb,
                     void *userdata, char const *product_dir);

#endif
