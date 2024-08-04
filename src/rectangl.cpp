#include "imbibe.h"

// #include "rectangle_element.h"
#include "rectangl.h"

#include "element.h"
#include "graphics.h"
#include "window.h"
#include "pixel.h"


rectangle_element::rectangle_element(int n_x1, int n_y1, int n_x2, int n_y2, int n_z, window & n_owner, pixel n_p):
  element(n_x1, n_y1, n_x2, n_y2, n_z, n_owner), m_p(n_p)
{
}


void rectangle_element::paint(graphics & g) const
{
  g.draw_rectangle(frame_x1(), frame_y1(), frame_x2(), frame_y2(), m_p);
}


