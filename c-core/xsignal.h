#ifndef XSIGNAL_H
#define XSIGNAL_H

#include <stdbool.h>

struct xsignal;

struct xsignal *xsignal_new(void);
int             xsignal_set(struct xsignal *);
int             xsignal_unset(struct xsignal *);
bool            xsignal_interrupted(struct xsignal *);
void            xsignal_del(struct xsignal *);

#endif
