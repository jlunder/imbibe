#ifndef __KEY_DISPATCHER_TASK_HH_INCLUDED
#define __KEY_DISPATCHER_TASK_HH_INCLUDED


#include "imbibe.h"

// #include "key_handler.h"
#include "key_hand.h"
#include "task.h"
#include "vector.h"


class key_dispatcher_task: public task
{
public:
  key_dispatcher_task();
  virtual void cycle();
  void add_handler(key_handler & k);
  void remove_handler(key_handler & k);

private:
  typedef vector<key_handler *> key_handler_p_list;

  key_handler_p_list m_key_handlers;
};


#endif //__KEY_DISPATCHER_TASK_HH_INCLUDED


