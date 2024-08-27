#ifndef __SCREEN_ELEMENT_H_INCLUDED
#define __SCREEN_ELEMENT_H_INCLUDED


#include "imbibe.h"

#include "tbm_element.h"
#include "window_element.h"


class screen_element: public window_element {
public:
  screen_element(): m_active(false) { }

  virtual void layout();
  virtual void poll() = 0;
  virtual bool handle_key(uint16_t key) = 0;
  virtual void animate(uint32_t delta_ms) = 0;
  virtual bool opaque() { return true; }

  bool active() { return m_active; }
  void activate() { m_active = true; }
  void deactivate() { m_active = false; }

private:
  bool m_active;
};


#endif // __SCREEN_ELEMENT_H_INCLUDED


