#include "imbibe.hh"

#include "stdlib.h"

// #include "key_dispatcher_task.hh"
#include "key_disp.hh"

// #include "key_handler.hh"
#include "key_hand.hh"
#include "task.hh"
#include "vector.hh"


extern bool key_avail();
#pragma aux key_avail = "mov ah, 011h"\
                        "int 016h"\
                        "mov al, 0"\
                        "jz @1"\
                        "mov al, 1"\
                        "@1:"\
                        value [al]\
                        modify exact [ax] nomemory;


extern uint16_t read_key();
#pragma aux read_key = "mov ah, 010h"\
                       "int 016h"\
                       value [ax]\
                       modify exact [ax] nomemory;


key_dispatcher_task::key_dispatcher_task()
{
}


void key_dispatcher_task::cycle()
{
  key_handler_p_list::iterator i;
  uint16_t c;

  while(key_avail())
  {
    c = read_key();
    if(((c & 0xFF) > 0) && ((c & 0xFF) < 128))
    {
      c = c & 0xFF;
    }
    else
    {
      c = (c >> 8) | 0x100;
    }
    for(i = m_key_handlers.begin(); i != m_key_handlers.end(); ++i)
    {
      if((*i)->handle(c)) break;
    }
  }
}


void key_dispatcher_task::add_handler(key_handler & k)
{
  m_key_handlers.insert(m_key_handlers.begin(), &k);
}


void key_dispatcher_task::remove_handler(key_handler & k)
{
  key_handler_p_list::iterator i;

  for(i = m_key_handlers.begin(); (i != m_key_handlers.end()) && (*i != &k); ++i);
  assert(i != m_key_handlers.end());
  m_key_handlers.erase(i);
}


