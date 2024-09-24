#ifndef SCAN_H
#define SCAN_H

#include <stdbool.h>

struct scan;
struct batch;

struct scan *scan_new(void);
void         scan_del(struct scan const *);
int          scan_setup(struct scan *, char const *dbfile, int port,
                        int num_threads, bool multi_hits, bool hmmer3_compat,
                        bool cache, void (*callback)(void *), void *userdata);
int          scan_run(struct scan *, struct batch *, char const *product_dir);
bool         scan_interrupted(struct scan const *);
int          scan_progress(struct scan const*);

#endif
