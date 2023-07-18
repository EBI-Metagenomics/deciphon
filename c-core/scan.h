#ifndef DECIPHON_SCAN_H
#define DECIPHON_SCAN_H

#include "api.h"
#include "scan_params.h"
#include "seq.h"
#include <stdbool.h>

struct dcp_scan;

DCP_API struct dcp_scan *dcp_scan_new(void);
DCP_API int dcp_scan_dial(struct dcp_scan *, int port);
DCP_API int dcp_scan_setup(struct dcp_scan *, struct dcp_scan_params);
DCP_API void dcp_scan_del(struct dcp_scan const *);

DCP_API int dcp_scan_run(struct dcp_scan *, char const *dbfile,
                         dcp_seq_next_fn *callb, void *userdata,
                         char const *product_dir);

#endif
