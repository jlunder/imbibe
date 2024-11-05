#ifndef __VIEWER_ELEMENT_H_INCLUDED
#define __VIEWER_ELEMENT_H_INCLUDED

#include "imbibe.h"

#include "imstring.h"
#include "screen_element.h"
#include "tbm.h"
#include "tweens.h"

class viewer_element : public screen_element {
public:
  virtual void layout(coord_t window_width, coord_t window_height);
  virtual void poll();
  virtual bool handle_key(key_code_t key);
  virtual void animate(anim_time_t delta_ms);
  virtual bool active() const;

  void activate(imstring const &view_label, imstring const &view_path);
  void deactivate();

  void enter_from_menu();
  void leave_to_menu();

private:
  imstring m_view_label;
  imstring m_view_path;
  tbm m_viewing;

  linear_tween<coord_t> m_transition_in_out;
  linear_tween<coord_t> m_scroll_y;

  bool m_active;
};

#endif // __VIEWER_ELEMENT_H_INCLUDED
