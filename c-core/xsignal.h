#ifndef XSIGNAL_H
#define XSIGNAL_H

#include <stdbool.h>

struct xsignal;

struct xsignal *xsignal_new(void);
bool            xsignal_interrupted(struct xsignal *);
void            xsignal_del(struct xsignal *);

#endif
