#ifndef __INTRO_ELEMENT_H_INCLUDED
#define __INTRO_ELEMENT_H_INCLUDED

#include "imbibe.h"

#include "bitmap_element.h"
#include "screen_element.h"
#include "tbm_element.h"
#include "tweens.h"

class intro_element : public screen_element {
public:
  intro_element();
  virtual ~intro_element() {}
  virtual void layout(coord_t window_width, coord_t window_height);
  virtual void poll();
  virtual bool handle_key(key_code_t key);
  virtual void animate(anim_time_t delta_ms);
  virtual bool active() const;
  virtual bool opaque() const;

  void set_capture(bitmap const &n_capture);

  void play_intro();

private:
  bitmap_element m_capture_background;
  tbm_element m_splash;
  tbm_element m_cover;
  linear_tween<uint8_t> m_logo_fade;
  linear_tween<coord_t> m_cover_y;
  bool m_active;
};

#endif // __INTRO_ELEMENT_H_INCLUDED
