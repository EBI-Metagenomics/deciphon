#ifndef DECIPHON_DEFER_RETURN_H
#define DECIPHON_DEFER_RETURN_H

#define defer_return(x)                                                        \
  do                                                                           \
  {                                                                            \
    rc = x;                                                                    \
    goto defer;                                                                \
  } while (0);

#endif
