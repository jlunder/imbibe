#include "imbibe.hh"

#include "rectangle_element.hh"

#include "element.hh"
#include "graphics.hh"
#include "window.hh"
#include "pixel.hh"


rectangle_element::rectangle_element(int n_x1, int n_y1, int n_x2, int n_y2, int n_z, window & n_owner, pixel n_p):
  element(n_x1, n_y1, n_x2, n_y2, n_z, n_owner), m_p(n_p)
{
}


void rectangle_element::paint(graphics & g) const
{
  g.draw_rectangle(frame_x1(), frame_y1(), frame_x2(), frame_y2(), m_p);
}


