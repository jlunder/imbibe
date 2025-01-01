#ifndef __DEBUG_H_INCLUDED
#define __DEBUG_H_INCLUDED

#include "base.h"

#define disable_logf(...)                                                      \
  do {                                                                         \
  } while (false)

#if !BUILD_DEBUG && BUILD_MSDOS
#define logf_any(...) disable_logf()
#else
#define logf_any(...) cprintf(__VA_ARGS__)
#endif

#define logf_sim(...) logf_any("SIM: " __VA_ARGS__)

#define abortf(...)                                                            \
  do {                                                                         \
    failsafe_textmode();                                                       \
    cprintf("fatal error: ");                                                  \
    cprintf(__VA_ARGS__);                                                      \
    cprintf("\r\n");                                                           \
    abort();                                                                   \
  } while (false)

#endif // __DEBUG_H_INCLUDED
