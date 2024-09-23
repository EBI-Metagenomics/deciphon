#if !defined(_POSIX_C_SOURCE) || _POSIX_C_SOURCE < 200809L
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#include <signal.h>

#include "error.h"
#include "xsignal.h"
#include <stdlib.h>

struct xsignal
{
  sigset_t old_mask;
  sigset_t new_sigmask;
};

struct xsignal *xsignal_new(void) { return malloc(sizeof(struct xsignal)); }

int xsignal_set(struct xsignal *x)
{
  sigemptyset(&x->new_sigmask);
  sigaddset(&x->new_sigmask, SIGINT);
  sigaddset(&x->new_sigmask, SIGTERM);
  return sigprocmask(SIG_BLOCK, &x->new_sigmask, &x->old_mask) ? DCP_ESIGNAL
                                                               : 0;
}

int xsignal_unset(struct xsignal *x)
{
  return sigprocmask(SIG_SETMASK, &x->old_mask, NULL) ? DCP_ESIGNAL : 0;
}

bool xsignal_interrupted(struct xsignal *x)
{
  sigset_t sigpend;
  int signal = 0;

  sigemptyset(&sigpend);
  sigpending(&sigpend);

  if (sigismember(&sigpend, SIGINT) || sigismember(&sigpend, SIGTERM))
  {
    if (sigwait(&x->new_sigmask, &signal) == 0) return true;
  }
  return false;
}

void xsignal_del(struct xsignal *x)
{
  if (x) free(x);
}
