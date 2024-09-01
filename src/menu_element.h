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
  // tbm_element m_header;
  // coord_t m_header_width;
  // coord_t m_header_height;
  // tbm_element m_footer;
  // coord_t m_footer_width;
  // coord_t m_footer_height;
  tbm_element m_background;
  coord_t m_scroll_height;
};

#endif // __MENU_ELEMENT_H_INCLUDED
