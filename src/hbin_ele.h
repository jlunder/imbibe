#ifndef __HBIN_ELEMENT_HH_INCLUDED
#define __HBIN_ELEMENT_HH_INCLUDED


#include "imbibe.h"

#include "element.h"
#include "graphics.h"
#include "hbin.h"
#include "window.h"


class hbin_element: public element
{
public:
  hbin_element(int n_x1, int n_y1, int n_x2, int n_y2, int n_z, window & n_owner, hbin & n_hb, int n_scroll_x = 0, int n_scroll_y = 0, int n_selection = 0);
  void paint(graphics & g) const;
  int scroll_x() const;
  int scroll_y() const;
  int selection() const;
  void set_scroll_pos(int x, int y);
  void set_selection(int n_selection);

private:
  hbin & m_hb;
  int m_scroll_x;
  int m_scroll_y;
  int m_selection;
};


inline int hbin_element::scroll_x() const
{
  return m_scroll_x;
}


inline int hbin_element::scroll_y() const
{
  return m_scroll_y;
}


inline int hbin_element::selection() const
{
  return m_selection;
}


#endif //__HBIN_ELEMENT_HH_INCLUDED


