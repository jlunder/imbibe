#ifndef __MENU_ELEMENT_H_INCLUDED
#define __MENU_ELEMENT_H_INCLUDED

#include "imbibe.h"

#include "rectangle_element.h"
#include "screen_element.h"
#include "tbm_element.h"

class menu_element : public screen_element {
public:
  menu_element();
  virtual ~menu_element();

  virtual void layout(coord_t window_width, coord_t window_height);
  virtual void poll();
  virtual bool handle_key(uint16_t key);
  virtual void animate(anim_time_t delta_ms);
  virtual bool active() const;

private:
  struct option_entry {
    rect hot;
    point viz;
    uint16_t offset;
  };

  struct menu_info {
    option_entry * entries;
    uint8_t data[];
  };

  // m_selecting_index;
  // m_deselecting_index;
  // m_selecting_transition;
  // m_deselecting_transition;
  // m_transition_right;

  //im_ptr<

  // tbm_element m_header;
  // coord_t m_header_width;
  // coord_t m_header_height;
  // tbm_element m_footer;
  // coord_t m_footer_width;
  // coord_t m_footer_height;

  tbm_element m_background;

  coord_t m_scroll_height;
#if 0
  class menu_option_element : public element {
  public:
    immut<tbm> m_selected;
    linear_tween<coord_t> m_hide_transition;
    linear_tween<coord_t> m_show_transition;

    void animate();

    void select();
    void unselect();
  };
  immut<menu_def> m_def;
  vector<menu_option_element> m_menu_options;

  immut<tbm_element> m_background;

  menu_state m_state;
#endif
};

#endif // __MENU_ELEMENT_H_INCLUDED
