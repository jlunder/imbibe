#ifndef __MENU_ELEMENT_H_INCLUDED
#define __MENU_ELEMENT_H_INCLUDED


#include "imbibe.h"

#include "element.h"


class menu;


class menu_element: public element
{
public:
  menu_element(coord_t n_x1, coord_t n_y1, coord_t n_x2, coord_t n_y2,
    coord_t n_z, window & n_owner, menu & n_m, coord_t n_scroll_x = 0,
    coord_t n_scroll_y = 0, coord_t n_selection = 0);
  virtual void paint(graphics & g);
  coord_t scroll_x() const { return m_scroll_x; }
  coord_t scroll_y() const { return m_scroll_y; }
  coord_t selection() const { return m_selection; }
  void set_scroll_pos(coord_t x, coord_t y);
  void set_selection(coord_t n_selection);

private:
  menu & m_m;
  coord_t m_scroll_x;
  coord_t m_scroll_y;
  coord_t m_selection;
};


#endif // __MENU_ELEMENT_H_INCLUDED


