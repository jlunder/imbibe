#ifndef __OUTRO_ELEMENT_H_INCLUDED
#define __OUTRO_ELEMENT_H_INCLUDED

#include "imbibe.h"

#include "screen_element.h"
#include "tbm_element.h"

class outro_element : public screen_element {
public:
  virtual void layout(coord_t window_width, coord_t window_height);
  virtual void poll();
  virtual bool handle_key(key_code_t key);
  virtual void animate(anim_time_t delta_ms);
  virtual bool active() const;

  void play_outro();

private:
  bool m_active;
};

#endif // __OUTRO_ELEMENT_H_INCLUDED
