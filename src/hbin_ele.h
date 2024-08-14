#ifndef __HBIN_ELEMENT_H_INCLUDED
#define __HBIN_ELEMENT_H_INCLUDED


#include "imbibe.h"

#include "element.h"
#include "graphics.h"
#include "hbin.h"
#include "window.h"


class hbin_element: public element
{
public:
  hbin_element(coord_t n_x1, coord_t n_y1, coord_t n_x2, coord_t n_y2,
    coord_t n_z, window & n_owner, hbin & n_hb, coord_t n_scroll_x = 0,
    coord_t n_scroll_y = 0, coord_t n_selection = 0);
  void paint(graphics & g);
  coord_t scroll_x() const { return m_scroll_x; }
  coord_t scroll_y() const { return m_scroll_y; }
  coord_t selection() const { return m_selection; }
  void set_scroll_pos(coord_t x, coord_t y);
  void set_selection(coord_t n_selection);

private:
  hbin & m_hb;
  coord_t m_scroll_x;
  coord_t m_scroll_y;
  coord_t m_selection;
};


#endif // __HBIN_ELEMENT_H_INCLUDED


