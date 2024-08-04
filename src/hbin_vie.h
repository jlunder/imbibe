#ifndef __HBIN_VIEW_HANDLER_HH_INCLUDED
#define __HBIN_VIEW_HANDLER_HH_INCLUDED


#include "imbibe.h"

// #include "key_handler.h"
#include "key_hand.h"
#include "hbin.h"
// #include "hbin_element.h"
#include "hbin_ele.h"


class hbin_view_handler: public key_handler
{
public:
  hbin_view_handler(hbin & n_hb, hbin_element & n_hbe);
  virtual bool handle(int c);
  void select_link(int selection);
  void scroll_to(int x, int y, bool down);

private:
  hbin & m_hb;
  hbin_element & m_hbe;
};


#endif //__HBIN_VIEW_HANDLER_HH_INCLUDED


