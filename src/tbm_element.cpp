#include "imbibe.h"

#include "tbm_element.h"

#include "graphics.h"
#include "termviz.h"


#define logf_tbm_element(...) disable_logf("TBM_ELEMENT: " __VA_ARGS__)


tbm_element::tbm_element()
  : element(), m_tbm(), m_fade(termviz::fade_steps - 1) {
}


tbm_element::~tbm_element() {
}


void tbm_element::set_tbm(unpacker const & tbm) {
  m_tbm = tbm;
  request_repaint();
}


void tbm_element::set_fade(uint8_t n_fade) {
  assert(n_fade < termviz::fade_steps);
  if (m_fade == n_fade) {
    return;
  }

  m_fade = n_fade;
  request_repaint();
}


void tbm_element::paint(graphics & g) {
  logf_tbm_element("paint element %p, bitmap %p, fade %d, corner %04X\n",
    this, m_b, m_fade, *m_b->data());
  if (!m_tbm.size()) {
    g.draw_rectangle(0, 0, frame_width(), frame_height(),
      termel::from('!', color::hi_green, color::red, true));
  } else if (m_fade >= termviz::fade_steps - 1) {
    g.draw_tbm(0, 0, m_tbm);
  } else {
    g.draw_tbm_fade(0, 0, m_tbm, m_fade);
  }
}

