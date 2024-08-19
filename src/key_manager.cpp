#include "imbibe.h"

#include "stdlib.h"

#include "key_manager.h"


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


bool key_manager::key_event_available() {
    return aux_key_manager__key_avail();
}


key_event_t key_manager::read_key_event() {
  return (key_event_t)aux_key_manager__read_key();
}


