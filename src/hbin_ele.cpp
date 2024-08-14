#include "imbibe.h"

// #include "hbin_element.h"
#include "hbin_ele.h"

#include "element.h"
#include "graphics.h"
#include "hbin.h"
#include "window.h"


hbin_element::hbin_element(coord_t n_x1, coord_t n_y1, coord_t n_x2,
    coord_t n_y2, coord_t n_z, window & n_owner, hbin & n_hb,
    coord_t n_scroll_x, coord_t n_scroll_y, coord_t n_selection)
  : element(), m_hb(n_hb),
    m_scroll_x(n_scroll_x), m_scroll_y(n_scroll_y), m_selection(n_selection) {
  set_frame(n_x1, n_y1, n_x2, n_y2, n_z);
  set_owner(n_owner);
}


void hbin_element::paint(graphics & g) {
  g.draw_bitmap(-m_scroll_x, -m_scroll_y, m_hb.background());
  g.draw_bitmap(-m_scroll_x + m_hb.link_x(m_selection),
    -m_scroll_y + m_hb.link_y(m_selection), m_hb.link_picture(m_selection));
}


void hbin_element::set_scroll_pos(coord_t x, coord_t y) {
  m_scroll_x = x;
  m_scroll_y = y;
  request_repaint();
}


void hbin_element::set_selection(coord_t n_selection) {
  assert(n_selection < m_hb.num_links());
  if(n_selection == m_selection) return;
  request_repaint(
    m_hb.link_x(m_selection) - m_scroll_x,
    m_hb.link_y(m_selection) - m_scroll_y,
    m_hb.link_x(m_selection) + m_hb.link_picture(m_selection).width()
      - m_scroll_x,
    m_hb.link_y(m_selection) + m_hb.link_picture(m_selection).height()
      - m_scroll_y);
  m_selection = n_selection;
  request_repaint(
    m_hb.link_x(m_selection) - m_scroll_x,
    m_hb.link_y(m_selection) - m_scroll_y,
    m_hb.link_x(m_selection) + m_hb.link_picture(m_selection).width()
      - m_scroll_x,
    m_hb.link_y(m_selection) + m_hb.link_picture(m_selection).height()
      - m_scroll_y);
}


