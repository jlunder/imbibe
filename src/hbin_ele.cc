#include "imbibe.hh"

// #include "hbin_element.hh"
#include "hbin_ele.hh"

#include "element.hh"
#include "graphics.hh"
#include "hbin.hh"
#include "window.hh"


hbin_element::hbin_element(int n_x1, int n_y1, int n_x2, int n_y2, int n_z, window & n_owner, hbin & n_hb, int n_scroll_x, int n_scroll_y, int n_selection):
  element(n_x1, n_y1, n_x2, n_y2, n_z, n_owner), m_hb(n_hb), m_scroll_x(n_scroll_x), m_scroll_y(n_scroll_y), m_selection(n_selection)
{
}


void hbin_element::paint(graphics & g) const
{
  g.draw_bitmap(-m_scroll_x, -m_scroll_y, m_hb.background());
  g.draw_bitmap(-m_scroll_x + m_hb.link_x(m_selection), -m_scroll_y + m_hb.link_y(m_selection), m_hb.link_picture(m_selection));
}


void hbin_element::set_scroll_pos(int x, int y)
{
  m_scroll_x = x;
  m_scroll_y = y;
  if(visible())
  {
    repaint();
  }
}


void hbin_element::set_selection(int n_selection)
{
  int x1;
  int y1;
  int x2;
  int y2;

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


