#ifndef __SCREEN_ELEMENT_H_INCLUDED
#define __SCREEN_ELEMENT_H_INCLUDED

#include "imbibe.h"

#include "keyboard.h"
#include "tbm_element.h"
#include "window_element.h"

class screen_element : public window_element {
public:
  virtual void layout(coord_t window_width, coord_t window_height) = 0;
  virtual void poll() = 0;
  virtual bool handle_key(key_code_t key) = 0;
  virtual void animate(anim_time_t delta_ms) = 0;
  virtual bool active() const = 0;
  virtual bool opaque() const { return true; }
};

#endif // __SCREEN_ELEMENT_H_INCLUDED
