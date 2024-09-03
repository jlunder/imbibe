#include "imbibe.h"

#include "rectangle_element.h"

#include "termviz.h"

rectangle_element::rectangle_element()
    : element(), m_brush(termel::from('R', color::hi_magenta, color::blue)) {}

void rectangle_element::paint(graphics *g) {
  g->draw_rectangle(frame(), m_brush);
}

void rectangle_element::set_brush(termel_t n_brush) {
  if (n_brush == m_brush) {
    return;
  }

  m_brush = n_brush;
  request_repaint();
}
