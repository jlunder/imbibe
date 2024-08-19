#ifndef __MAIN_ELEMENT_H_INCLUDED
#define __MAIN_ELEMENT_H_INCLUDED


#include "imbibe.h"

// #include "window_element.h"
#include "window_element.h"


class main_element : public window_element {
public:
  main_element();
  virtual ~main_element();

public:
  virtual void animate(uint32_t delta_ms);
  virtual bool handle_key(uint16_t key);

  void animate_intro(uint32_t delta_ms);
  void animate_main_menu(uint32_t delta_ms);
  void animate_outro(uint32_t delta_ms);

  void enter_main_menu();
  void enter_outro();

private:
  enum state_t {
    st_intro,
    st_main_menu,
    st_outro
  };

  state_t m_state;
  uint32_t m_anim_ms;

  window_element m_frame;
  window_element m_scroll;

  // intro_element
  // outro_element
  // main_menu_element
  // viewer_element
  //
};


#endif // __MAIN_ELEMENT_H_INCLUDED

