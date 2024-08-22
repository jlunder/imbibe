#include "imbibe.h"

#include "stdlib.h"

#include "keyboard.h"


#define logf_key_manager(...) disable_logf("KEY_MANAGER: " __VA_ARGS__)


extern bool aux_key_manager__key_avail();
extern uint16_t aux_key_manager__read_key();


#if !defined(SIMULATE)

#pragma aux aux_key_manager__key_avail = \
  "mov ah, 011h"\
  "int 016h"\
  "mov al, 0"\
  "jz @1"\
  "mov al, 1"\
  "@1:"\
  value [al]\
  modify exact [ax] nomemory;

#pragma aux aux_key_manager__read_key = \
  "mov ah, 010h"\
  "int 016h"\
  value [ax]\
  modify exact [ax] nomemory;

#endif


bool keyboard::key_event_available() {
    return aux_key_manager__key_avail();
}


key_code_t keyboard::read_key_event() {
  uint16_t k = aux_key_manager__read_key();
  if ((k & 0xFF) != 0) {
    return (key_code_t)(k & 0xFF);
  } else {
    return (key_code_t)(0x100 | (k >> 8));
  }
}


