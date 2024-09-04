#include "imbibe.h"

#include "bitmap_element.h"

#include "bitmap.h"
#include "element.h"
#include "graphics.h"
#include "termviz.h"

#define logf_bitmap_element(...) disable_logf("BITMAP_ELEMENT: " __VA_ARGS__)

bitmap_element::bitmap_element()
    : element(), m_b(), m_fade(termviz::fade_steps - 1) {}

void bitmap_element::set_b(bitmap const &n_b) {
  m_b = n_b;
  request_repaint();
}

void bitmap_element::set_fade(uint8_t n_fade) {
  assert(n_fade < termviz::fade_steps);
  if (m_fade == n_fade) {
    return;
  }

  m_fade = n_fade;
  request_repaint();
}

void bitmap_element::paint(graphics *g) {
  logf_bitmap_element("paint element %p, fade %d, corner %04X\n", this, m_fade,
                      *m_b.data());
#ifndef NDEBUG
  g->draw_rectangle(0, 0, frame().width(), frame().height(),
                    termel::from('!', color::hi_blue, color::red, true));
#endif
  if (m_fade >= termviz::fade_steps - 1) {
    g->draw_bitmap(0, 0, m_b);
  } else if (m_fade == 0) {
    g->draw_rectangle(0, 0, m_b.width(), m_b.height(), termel::from(' ', 0, 0));
  } else {
    g->draw_bitmap_fade(0, 0, m_b, m_fade);
  }
}
