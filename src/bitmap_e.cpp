#include "imbibe.h"

//#include "bitmap_element.h"
#include "bitmap_e.h"

#include "bitmap.h"
#include "element.h"
#include "graphics.h"


// #undef logf
// #define logf cprintf


bitmap_element::bitmap_element()
  : element(), m_b(NULL)
{
}


bitmap_element::~bitmap_element()
{
  if(m_b) {
    delete m_b;
  }
}


void bitmap_element::set_b(bitmap * n_b) {
  if(m_b) {
    delete m_b;
  }
  m_b = n_b;
  repaint();
}


void bitmap_element::paint(graphics & g) const
{
  logf("paint element %p, bitmap %p, corner %04X\n", this, m_b,
    *(uint16_t*)m_b->data());
  if(m_b) {
    g.draw_bitmap(frame_x1(), frame_y1(), *m_b);
  }
}


