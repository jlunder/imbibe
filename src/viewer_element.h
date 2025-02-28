#ifndef __VIEWER_ELEMENT_H_INCLUDED
#define __VIEWER_ELEMENT_H_INCLUDED

#include "imbibe.h"

#include "imstring.h"
#include "screen_element.h"
#include "tbm.h"
#include "termviz.h"
#include "tweens.h"

class viewer_element : public screen_element {
public:
  virtual void layout(coord_t window_width, coord_t window_height);
  virtual void poll();
  virtual bool handle_key(key_code_t key);
  virtual void animate(anim_time_t delta_ms);
  virtual bool active() const;
  virtual bool opaque() const;
  virtual void paint(graphics *g);

  void activate(imstring const &resource);
  void deactivate();

  void enter_from_menu_or_submenu();
  void leave_to_menu_or_submenu();

private:
  tbm m_viewing;
  termel_t m_background;

  coord_t m_page_jump;
  coord_t m_scroll_height;
  coord_t m_view_height;
  coord_t m_scroll_y_target;
  coord_t m_scroll_y_target_last;

  linear_tween<coord_t> m_transition_in_out;
  linear_tween<coord_t> m_scroll_y;
  point m_viewing_offset;

  bool m_active;
};

#endif // __VIEWER_ELEMENT_H_INCLUDED
