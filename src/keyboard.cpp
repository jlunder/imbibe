#include "imbibe.h"

#include "stdlib.h"

#include "keyboard.h"


#define logf_key_manager(...) disable_logf("KEY_MANAGER: " __VA_ARGS__)


#if !defined(SIMULATE)


bool keyboard::key_event_available() {
  extern bool bios_key_event_available();
  #pragma aux bios_key_event_available = \
    "   mov     ah, 011h          " \
    "   int     016h              " \
    "   mov     al, 0             " \
    "   jz      @1                " \
    "   mov     al, 1             " \
    "@1:                          " \
    modify nomemory [ax] \
    value [al]

  return bios_key_event_available();
}


key_code_t keyboard::read_key_event() {
  extern uint16_t bios_read_key_event();
  #pragma aux bios_read_key_event = \
    "   mov     ah, 010h          " \
    "   int     016h              " \
    modify nomemory [ax] \
    value [ax];

  uint16_t k = bios_read_key_event();
  if ((k & 0xFF) != 0) {
    return (key_code_t)(k & 0xFF);
  } else {
    return (key_code_t)(0x100 | (k >> 8));
  }
}


#endif


