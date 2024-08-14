#include "imbibe.h"

// #include "menu_element.h"
#include "menu_ele.h"

#include "bitmap.h"
#include "element.h"
#include "graphics.h"
#include "menu.h"
#include "window.h"
#include "pixel.h"


menu_element::menu_element(
    coord_t n_x1, coord_t n_y1, coord_t n_x2, coord_t n_y2, coord_t n_z,
    window & n_owner, menu & n_m, coord_t n_scroll_x, coord_t n_scroll_y,
    coord_t n_selection)
  : element(), m_m(n_m), m_scroll_x(n_scroll_x), m_scroll_y(n_scroll_y),
    m_selection(n_selection) {
  assert(m_selection < m_m.num_links());
  set_frame(n_x1, n_y1, n_x2, n_y2, n_z);
  set_owner(n_owner);
}


void menu_element::paint(graphics & g) {
  int i;

  for(i = 0; i < m_selection; ++i) {
    g.draw_bitmap(m_m.link_x(i), m_m.link_y(i), m_m.link_normal_picture(i));
  }
  g.draw_bitmap(m_m.link_x(m_selection), m_m.link_y(m_selection),
    m_m.link_selected_picture(m_selection));
  for(i = m_selection + 1; i < m_m.num_links(); ++i) {
    g.draw_bitmap(m_m.link_x(i), m_m.link_y(i), m_m.link_normal_picture(i));
  }
}


void menu_element::set_scroll_pos(coord_t x, coord_t y) {
  m_scroll_x = x;
  m_scroll_y = y;
  if(visible()) {
    request_repaint();
  }
}


void menu_element::set_selection(coord_t n_selection) {
  assert(n_selection < m_m.num_links());
  if(n_selection == m_selection) return;
  m_selection = n_selection;
  if(visible()) {
    request_repaint(m_m.link_x(m_selection) - m_scroll_x,
      m_m.link_y(m_selection) - m_scroll_y,
      m_m.link_x(m_selection) + m_m.link_width(m_selection) - m_scroll_x,
      m_m.link_y(m_selection) + m_m.link_height(m_selection) - m_scroll_y);
  }
}


