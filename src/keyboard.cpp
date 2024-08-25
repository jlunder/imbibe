#include "imbibe.h"

#include "stdlib.h"

#include "keyboard.h"


#define logf_key_manager(...) disable_logf("KEYBOARD: " __VA_ARGS__)


#if !defined(SIMULATE)


extern bool asm_bios_key_event_available();
#pragma aux asm_bios_key_event_available = \
  "   mov     ah, 011h          " \
  "   int     016h              " \
  "   mov     al, 0             " \
  "   jz      @1                " \
  "   mov     al, 1             " \
  "@1:                          " \
  modify nomemory [ax] \
  value [al]


bool keyboard::key_event_available() {
  return asm_bios_key_event_available();
}


extern uint16_t asm_bios_read_key_event();
#pragma aux asm_bios_read_key_event = \
  "   mov     ah, 010h          " \
  "   int     016h              " \
  modify nomemory [ax] \
  value [ax];


key_code_t keyboard::read_key_event() {
  uint16_t k = asm_bios_read_key_event();
  if ((k & 0xFF) != 0) {
    return (key_code_t)(k & 0xFF);
  } else {
    return (key_code_t)(0x100 | (k >> 8));
  }
}


#endif


