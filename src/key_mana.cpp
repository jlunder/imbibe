#include "imbibe.h"

#include "stdlib.h"

// #include "key_manager.h"
#include "key_mana.h"

// #include "key_handler.h"
#include "key_hand.h"
#include "task.h"
#include "vector.h"


#undef logf
#define logf cprintf


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


key_manager::key_handler_p_list key_manager::s_key_handlers;


void key_manager::dispatch_keys() {
  key_handler_p_list::iterator i;
  uint16_t c;

  while(aux_key_manager__key_avail())   {
    c = aux_key_manager__read_key();
    if(((c & 0xFF) > 0) && ((c & 0xFF) < 128)) {
      c = c & 0xFF;
    } else {
      c = (c >> 8) | 0x100;
    }
    for(i = s_key_handlers.begin(); i != s_key_handlers.end(); ++i) {
      logf("calling key_handler %p\n", *i);
      if((*i)->handle_key(c)) {
        break;
      }
    }
  }
}


void key_manager::add_handler(key_handler & k)
{
  s_key_handlers.insert(s_key_handlers.begin(), &k);
  logf("full list:\n");
  for (uint16_t i = 0; i < s_key_handlers.size(); ++i) {
    logf("  %u: %p\n", i, s_key_handlers[i]);
  }
}


void key_manager::remove_handler(key_handler & k)
{
  key_handler_p_list::iterator i;

  for(i = s_key_handlers.begin(); (i != s_key_handlers.end()) && (*i != &k); ++i);
  assert(i != s_key_handlers.end());
  s_key_handlers.erase(i);
}


