#ifndef __MENU_ELEMENT_HH_INCLUDED
#define __MENU_ELEMENT_HH_INCLUDED


#include "imbibe.h"

#include "bitmap.h"
#include "element.h"
#include "graphics.h"
#include "menu.h"
#include "window.h"
#include "pixel.h"


class menu_element: public element
{
public:
  menu_element(int16_t n_x1, int16_t n_y1, int16_t n_x2, int16_t n_y2, int16_t n_z, window & n_owner, menu & n_m, int16_t n_scroll_x = 0, int16_t n_scroll_y = 0, int16_t n_selection = 0);
  virtual void paint(graphics & g) const;
  int16_t scroll_x() const { return m_scroll_x; }
  int16_t scroll_y() const { return m_scroll_y; }
  int16_t selection() const { return m_selection; }
  void set_scroll_pos(int16_t x, int16_t y);
  void set_selection(int16_t n_selection);

private:
  menu & m_m;
  int16_t m_scroll_x;
  int16_t m_scroll_y;
  int16_t m_selection;
};


#endif //__MENU_ELEMENT_HH_INCLUDED


