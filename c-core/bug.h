#ifndef BUG_H
#define BUG_H

#include "compiler.h"

/* COPIED FROM LINUX KERNEL
 * Don't use BUG() or BUG_ON() unless there's really no way out; one
 * example might be detecting data structure corruption in the middle
 * of an operation that can't be backed out of.  If the (sub)system
 * can somehow continue operating, perhaps with reduced functionality,
 * it's probably not BUG-worthy.
 *
 * If you're tempted to BUG(), think again:  is completely giving up
 * really the *only* solution?  There are usually better options, where
 * users don't need to reboot ASAP and can mostly shut down cleanly.
 */
#define BUG()                                                                  \
  do                                                                           \
  {                                                                            \
    bug(__FILE__, __LINE__, __func__);                                         \
  } while (0)

#define BUG_ON(condition)                                                      \
  do                                                                           \
  {                                                                            \
    if (unlikely(condition)) BUG();                                            \
  } while (0)

void bug(char const *file, int line, char const *func);

#endif
