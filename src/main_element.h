#ifndef __MAIN_ELEMENT_H_INCLUDED
#define __MAIN_ELEMENT_H_INCLUDED


#include "imbibe.h"

#include "animator.h"
#include "bitmap_element.h"
#include "rectangle_element.h"
#include "tweens.h"
#include "window_element.h"


class main_element : public window_element {
public:
  main_element();
  virtual ~main_element();

public:
  virtual void layout();
  virtual void animate(uint32_t delta_ms);
  virtual bool handle_key(uint16_t key);

private:
  enum state_t {
    st_init,
    st_intro,
    st_main_menu,
    st_outro
  };

  static const animator::key_t k_logo_fade = 1;
  static const animator::key_t k_cover_scroll = 2;
  static const animator::key_t k_submenu_slide = 3;

  state_t m_state;

  // animator m_animator;

  linear_tween<uint8_t> m_logo_fade;
  linear_tween<coord_t> m_cover_scroll;
  timer_tween m_anim_timer;
  // tweens::quadratic<coord_t> m_submenu_slide;

  uint8_t m_prop_fade;
  coord_t m_prop_submenu_slide;
  coord_t m_prop_cover_scroll;

  window_element m_frame;

  rectangle_element m_logo_background;
  bitmap_element m_logo;
  // bitmap_element m_cover;
  rectangle_element m_cover;

  window_element m_menu;
  // bitmap_element m_menu_header;
  rectangle_element m_menu_header;
  // bitmap_element m_menu_footer;
  rectangle_element m_menu_footer;
  rectangle_element m_menu_background;

  // window_element m_submenu;
  // bitmap_element m_submenu_header;
  // bitmap_element m_submenu_footer;
  // rectangle_element m_submenu_background;

  void animate_intro(uint32_t delta_ms);
  void animate_main_menu(uint32_t delta_ms);
  void animate_outro(uint32_t delta_ms);

  void enter_intro();
  void enter_main_menu();
  void enter_outro();
};


#endif // __MAIN_ELEMENT_H_INCLUDED

