#include "imbibe.h"

#include "bitmap_element.h"

#include "bitmap.h"
#include "element.h"
#include "graphics.h"
#include "termviz.h"

#define logf_bitmap_element(...) disable_logf("BITMAP_ELEMENT: " __VA_ARGS__)

bitmap_element::bitmap_element()
    : element(), m_b(NULL), m_fade(termviz::fade_steps - 1) {}

bitmap_element::~bitmap_element() {}

void bitmap_element::set_b(im_ptr<bitmap> n_b) {
  if (m_b == n_b) {
    return;
  }

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
  logf_bitmap_element("paint element %p, bitmap %p, fade %d, corner %04X\n",
                      this, m_b, m_fade, *m_b->data());
  if (!m_b) {
    g->draw_rectangle(0, 0, frame_width(), frame_height(),
                     termel::from('!', color::hi_blue, color::red, true));
  } else if (m_fade >= termviz::fade_steps - 1) {
    g->draw_bitmap(0, 0, *m_b);
  } else if (m_fade == 0) {
    g->draw_rectangle(0, 0, m_b->width(), m_b->height(),
                     termel::from(' ', 0, 0));
  } else {
    g->draw_bitmap_fade(0, 0, *m_b, m_fade);
  }
}
