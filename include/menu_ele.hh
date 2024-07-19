#ifndef __MENU_ELEMENT_HH_INCLUDED
#define __MENU_ELEMENT_HH_INCLUDED


#include "bitmap.hh"
#include "element.hh"
#include "graphics.hh"
#include "menu.hh"
#include "window.hh"
#include "pixel.hh"


class menu_element: public element
{
public:
  menu_element(int n_x1, int n_y1, int n_x2, int n_y2, int n_z, window & n_owner, menu & n_m, int n_scroll_x = 0, int n_scroll_y = 0, n_selection = 0);
  virtual void paint(graphics & g) const;
  int scroll_x() const;
  int scroll_y() const;
  int selection() const;
  void set_scroll_pos(int x, int y);
  void set_selection(int n_selection);

private:
  menu & m_m;
  int m_scroll_x;
  int m_scroll_y;
  int m_selection;
};


#endif //__MENU_ELEMENT_HH_INCLUDED


