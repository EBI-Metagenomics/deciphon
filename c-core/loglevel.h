#ifndef LOGLEVEL_H
#define LOGLEVEL_H

enum
{
  LOGLEVEL_DEBUG = 0,
  LOGLEVEL_INFO = 1,
  LOGLEVEL_ERROR = 2,
  LOGLEVEL_NONE = 3,
};

int loglevel(void);

#endif
