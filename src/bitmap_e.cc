#include "imbibe.hh"

#include "bitmap_element.hh"

#include "bitmap.hh"
#include "element.hh"
#include "graphics.hh"


bitmap_element::bitmap_element(int n_x1, int n_y1, int n_x2, int n_y2, int n_z, window & n_owner, bitmap * n_b):
  element(n_x1, n_y1, n_x2, n_y2, n_z, n_owner), m_b(n_b)
{
}


bitmap_element::~bitmap_element()
{
  delete m_b;
}


void bitmap_element::paint(graphics & g) const
{
  g.draw_bitmap(0, 0, *m_b);
}


