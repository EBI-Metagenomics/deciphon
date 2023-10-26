#ifndef API_H
#define API_H

/* clang-format off */
#ifdef DECIPHON_STATIC_DEFINE
#  define API
#else
#  ifdef deciphon_EXPORTS /* We are building this library */
#    ifdef _WIN32
#      define API __declspec(dllexport)
#    else
#      define API __attribute__((visibility("default")))
#    endif
#  else /* We are using this library */
#    ifdef _WIN32
#      define API __declspec(dllimport)
#    else
#      define API __attribute__((visibility("default")))
#    endif
#  endif
#endif
/* clang-format on */

#endif
