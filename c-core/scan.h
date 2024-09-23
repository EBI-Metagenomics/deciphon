#ifndef SCAN_H
#define SCAN_H

#include <stdbool.h>

struct scan;
struct batch;

struct scan *scan_new(void);
void         scan_del(struct scan const *);

int  scan_setup(struct scan *, int port, int num_threads, bool multi_hits, bool hmmer3_compat);
int  scan_open(struct scan *, char const *dbfile);
int  scan_close(struct scan *);
int  scan_run(struct scan *, struct batch *, char const *product_dir, bool (*interrupt)(void *), void *userdata);
bool scan_interrupted(struct scan const *);
int  scan_progress(struct scan const*);

#endif
