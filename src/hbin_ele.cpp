#include "imbibe.h"

// #include "hbin_element.h"
#include "hbin_ele.h"

#include "element.h"
#include "graphics.h"
#include "hbin.h"
#include "window.h"


hbin_element::hbin_element(int16_t n_x1, int16_t n_y1, int16_t n_x2,
    int16_t n_y2, int16_t n_z, window & n_owner, hbin & n_hb,
    int16_t n_scroll_x, int16_t n_scroll_y, int16_t n_selection)
  : element(), m_hb(n_hb),
    m_scroll_x(n_scroll_x), m_scroll_y(n_scroll_y), m_selection(n_selection)
{
  set_frame(n_x1, n_y1, n_x2, n_y2, n_z);
  set_owner(n_owner);
}


void hbin_element::paint(graphics & g) const
{
  g.draw_bitmap(-m_scroll_x, -m_scroll_y, m_hb.background());
  g.draw_bitmap(-m_scroll_x + m_hb.link_x(m_selection),
    -m_scroll_y + m_hb.link_y(m_selection), m_hb.link_picture(m_selection));
}


void hbin_element::set_scroll_pos(int16_t x, int16_t y)
{
  m_scroll_x = x;
  m_scroll_y = y;
  if(visible())
  {
    repaint();
  }
}


void hbin_element::set_selection(int16_t n_selection)
{
  int16_t x1;
  int16_t y1;
  int16_t x2;
  int16_t y2;

  assert(n_selection < m_hb.num_links());
  if(n_selection == m_selection) return;
  if(visible())
  {
    x1 = m_hb.link_x(m_selection) - m_scroll_x;
    y1 = m_hb.link_y(m_selection) - m_scroll_y;
    x2 = m_hb.link_x(m_selection) + m_hb.link_picture(m_selection).width() - m_scroll_x;
    y2 = m_hb.link_y(m_selection) + m_hb.link_picture(m_selection).height() - m_scroll_y;
    repaint(x1, y1, x2, y2);
    m_selection = n_selection;
    x1 = m_hb.link_x(m_selection) - m_scroll_x;
    y1 = m_hb.link_y(m_selection) - m_scroll_y;
    x2 = m_hb.link_x(m_selection) + m_hb.link_picture(m_selection).width() - m_scroll_x;
    y2 = m_hb.link_y(m_selection) + m_hb.link_picture(m_selection).height() - m_scroll_y;
    repaint(x1, y1, x2, y2);
  }
  else
  {
    m_selection = n_selection;
  }
}


