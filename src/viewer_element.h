#ifndef __VIEWER_ELEMENT_H_INCLUDED
#define __VIEWER_ELEMENT_H_INCLUDED


#include "imbibe.h"

#include "screen_element.h"


class viewer_element: public screen_element {
public:
  virtual void poll();
  virtual bool handle_key(uint16_t key);
  virtual void animate(uint32_t delta_ms);
};


#endif // __VIEWER_ELEMENT_H_INCLUDED

