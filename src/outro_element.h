#ifndef __OUTRO_ELEMENT_H_INCLUDED
#define __OUTRO_ELEMENT_H_INCLUDED

#include "imbibe.h"

#include "rectangle_element.h"
#include "screen_element.h"
#include "tbm_element.h"
#include "tweens.h"

class outro_element : public screen_element {
public:
  outro_element();
  virtual void layout(coord_t window_width, coord_t window_height);
  virtual void poll();
  virtual bool handle_key(key_code_t key);
  virtual void animate(anim_time_t delta_ms);
  virtual bool active() const;
  virtual bool opaque() const;

  void play_outro();
  void finish_outro();

private:
  tbm_element m_goodbye;
  tbm_element m_logo;
  rectangle_element m_background;
  linear_tween<coord_t> m_goodbye_y;
  bool m_active;
};

#endif // __OUTRO_ELEMENT_H_INCLUDED
