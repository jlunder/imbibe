#ifndef __KEY_MANAGER_H_INCLUDED
#define __KEY_MANAGER_H_INCLUDED


#include "imbibe.h"

// #include "key_handler.h"
#include "key_hand.h"
#include "task.h"
#include "vector.h"


class key_manager
{
public:
  static void dispatch_keys();
  static void add_handler(key_handler & k);
  static void remove_handler(key_handler & k);

private:
  typedef vector<key_handler *> key_handler_p_list;

  static key_handler_p_list s_key_handlers;
};


#endif // __KEY_MANAGER_H_INCLUDED


