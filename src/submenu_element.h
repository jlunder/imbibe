#ifndef __SUBMENU_ELEMENT_H_INCLUDED
#define __SUBMENU_ELEMENT_H_INCLUDED

#include "imbibe.h"

#include "imstring.h"
#include "map.h"
#include "screen_element.h"
#include "tbm_element.h"
#include "tweens.h"
#include "vector.h"

class submenu_element : public screen_element {
public:
  virtual void layout(coord_t window_width, coord_t window_height);
  virtual void poll();
  virtual bool handle_key(key_code_t key);
  virtual void animate(anim_time_t delta_ms);
  virtual bool active() const;
  virtual void paint(graphics *g);

  void activate(imstring const &cfg_path);
  void deactivate();

private:
  struct submenu_option {
    point label_pos;
    point unselected_pos;
    point selected_pos;
    imstring label;
    imstring view_path;
  };

  struct submenu {
    // config
    immutable config;
    tbm menu_header;
    tbm menu_footer;
    tbm option_unselected_background;
    tbm option_selected_background;
    point option_selected_offset;
    point option_label_offset;
    attribute_t option_unselected_label_attribute;
    attribute_t option_selected_label_attribute;

    // computed values
    point menu_header_pos;
    point menu_footer_pos;
    point option_origin;
    coord_t option_height;
    coord_t menu_height;
    segsize_t page_jump;

    vector<submenu_option> options;
  };

  vector<submenu> m_submenus;
  map<imstring, submenu *> m_submenus_by_name;

  immutable m_menu_config;
  submenu *m_submenu;

  segsize_t m_selected_option;
  segsize_t m_last_selected_option;
  segsize_t m_unselected_option;

  linear_tween<coord_t> m_transition_in_out;
  linear_tween<coord_t> m_scroll_y;
  linear_tween<coord_t> m_hide_option_transition;
  linear_tween<coord_t> m_show_option_transition;

  bool m_active;

  bool try_unpack_menu_config();
  bool try_unpack_submenu_config(imstring const &cfg_path, submenu *out_submenu);

  void enter_from_menu();
  void leave_to_menu();
  void enter_from_viewer();
  void leave_to_viewer();
};

#endif // __SUBMENU_ELEMENT_H_INCLUDED
