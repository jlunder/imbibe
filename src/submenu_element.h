#ifndef __SUBMENU_ELEMENT_H_INCLUDED
#define __SUBMENU_ELEMENT_H_INCLUDED


#include "imbibe.h"

#include "screen_element.h"
#include "tbm_element.h"


class submenu_element: public screen_element {
public:
  virtual void poll();
  virtual bool handle_key(uint16_t key);
  virtual void animate(uint32_t delta_ms);
};


#endif // __SUBMENU_ELEMENT_H_INCLUDED


