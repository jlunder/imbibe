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
  hbin_element(int16_t n_x1, int16_t n_y1, int16_t n_x2, int16_t n_y2,
    int16_t n_z, window & n_owner, hbin & n_hb, int16_t n_scroll_x = 0,
    int16_t n_scroll_y = 0, int16_t n_selection = 0);
  void paint(graphics & g) const;
  int16_t scroll_x() const { return m_scroll_x; }
  int16_t scroll_y() const { return m_scroll_y; }
  int16_t selection() const { return m_selection; }
  void set_scroll_pos(int16_t x, int16_t y);
  void set_selection(int16_t n_selection);

private:
  hbin & m_hb;
  int16_t m_scroll_x;
  int16_t m_scroll_y;
  int16_t m_selection;
};


#endif //__HBIN_ELEMENT_H_INCLUDED


