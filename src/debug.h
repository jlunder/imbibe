#ifndef __DEBUG_H_INCLUDED
#define __DEBUG_H_INCLUDED

#include "base.h"

#define logf_any(...) cprintf(__VA_ARGS__)
#define disable_logf(...)                                                      \
  do {                                                                         \
  } while (false)
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
