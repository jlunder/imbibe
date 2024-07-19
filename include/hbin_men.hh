#ifndef __HBIN_MENU_HANDLER_HH_INCLUDED
#define __HBIN_MENU_HANDLER_HH_INCLUDED


#include "key_handler.hh"
#include "hbin.hh"
#include "hbin_element.hh"


class hbin_menu_handler: public key_handler
{
public:
  hbin_menu_handler(hbin & n_hb, hbin_element & n_hbe);
  virtual bool handle(int c);

private:
  hbin & m_hb;
  hbin_element & m_hbe;
};


#endif //__HBIN_MENU_HANDLER_HH_INCLUDED


