#ifndef __MENU_HANDLER_HH_INCLUDED
#define __MENU_HANDLER_HH_INCLUDED


#include "imbibe.hh"

// #include "key_handler.hh"
#include "key_hand.hh"
#include "menu.hh"
// #include "menu_element.hh"
#include "menu_ele.hh"


class menu_handler: public key_handler
{
public:
  menu_handler(menu & n_m, menu_element & n_me);
  virtual bool handle(int c);

private:
  menu & m_m;
  menu_element & m_me;
};


#endif //__MENU_HANDLER_HH_INCLUDED


