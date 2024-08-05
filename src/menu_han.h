#ifndef __MENU_HANDLER_H_INCLUDED
#define __MENU_HANDLER_H_INCLUDED


#include "imbibe.h"

// #include "key_handler.h"
#include "key_hand.h"
#include "menu.h"
// #include "menu_element.h"
#include "menu_ele.h"


class menu_handler: public key_handler
{
public:
  menu_handler(menu & n_m, menu_element & n_me);
  virtual bool handle(int c);

private:
  menu & m_m;
  menu_element & m_me;
};


#endif // __MENU_HANDLER_H_INCLUDED


