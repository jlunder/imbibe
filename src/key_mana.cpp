#include "imbibe.h"

#include "stdlib.h"

// #include "key_manager.h"
#include "key_mana.h"

// #include "key_handler.h"
#include "key_hand.h"
#include "task.h"
#include "vector.h"


extern bool key_manager_key_avail();
#pragma aux key_manager_key_avail = \
  "mov ah, 011h"\
  "int 016h"\
  "mov al, 0"\
  "jz @1"\
  "mov al, 1"\
  "@1:"\
  value [al]\
  modify exact [ax] nomemory;


extern uint16_t key_manager_read_key();
#pragma aux key_manager_read_key = \
  "mov ah, 010h"\
  "int 016h"\
  value [ax]\
  modify exact [ax] nomemory;


key_manager::key_handler_p_list key_manager::s_key_handlers;


void key_manager::dispatch_keys() {
  key_handler_p_list::iterator i;
  uint16_t c;

  while(key_manager_key_avail())   {
    c = key_manager_read_key();
    if(((c & 0xFF) > 0) && ((c & 0xFF) < 128)) {
      c = c & 0xFF;
    } else {
      c = (c >> 8) | 0x100;
    }
    for(i = s_key_handlers.begin(); i != s_key_handlers.end(); ++i) {
      if((*i)->handle_key(c)) {
        break;
      }
    }
  }
}


void key_manager::add_handler(key_handler & k)
{
  s_key_handlers.insert(s_key_handlers.begin(), &k);
}


void key_manager::remove_handler(key_handler & k)
{
  key_handler_p_list::iterator i;

  for(i = s_key_handlers.begin(); (i != s_key_handlers.end()) && (*i != &k); ++i);
  assert(i != s_key_handlers.end());
  s_key_handlers.erase(i);
}


