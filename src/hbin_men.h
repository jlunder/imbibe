#ifndef __HBIN_MENU_HANDLER_H_INCLUDED
#define __HBIN_MENU_HANDLER_H_INCLUDED


#include "imbibe.h"

// #include "key_handler.h"
#include "key_hand.h"
#include "hbin.h"
// #include "hbin_element.h"
#include "hbin_ele.h"


class hbin_menu_handler: public key_handler
{
public:
  hbin_menu_handler(hbin & n_hb, hbin_element & n_hbe);
  virtual bool handle(int c);

private:
  hbin & m_hb;
  hbin_element & m_hbe;
};


#endif // __HBIN_MENU_HANDLER_H_INCLUDED


