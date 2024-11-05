#ifndef __MENU_ELEMENT_H_INCLUDED
#define __MENU_ELEMENT_H_INCLUDED

#include "imbibe.h"

#include "screen_element.h"
#include "tbm.h"
#include "tweens.h"

class menu_element : public screen_element {
public:
  menu_element();
  virtual ~menu_element() {}

  virtual void layout(coord_t window_width, coord_t window_height);
  virtual void poll();
  virtual bool handle_key(key_code_t key);
  virtual void animate(anim_time_t delta_ms);
  virtual bool active() const;
  virtual void paint(graphics *g);

  void activate();

private:
  struct menu_option {
    rect hot;
    point selected_pos;
    tbm selected_overlay;
    imstring config;
    linear_tween<coord_t> hide_transition;
    linear_tween<coord_t> show_transition;
  };

  vector<menu_option> m_options;
  linear_tween<coord_t> m_scroll_y;
  tbm m_background;
  segsize_t m_selected_option;
  segsize_t m_last_selected_option;
};

#endif // __MENU_ELEMENT_H_INCLUDED
