#include "imbibe.hh"

#include "menu_element.hh"

#include "bitmap.hh"
#include "element.hh"
#include "graphics.hh"
#include "window.hh"
#include "pixel.hh"

#include "menu_element.ii"

#include "bitmap.ii"
#include "element.ii"
#include "graphics.ii"
#include "window.ii"
#include "pixel.ii"


menu_element::menu_element(int n_x1, int n_y1, int n_x2, int n_y2, int n_z, window & n_owner, menu & n_m, int n_scroll_x, int n_scroll_y, int n_selection):
  element(n_x1, n_y1, n_x2, n_y2, n_z, n_owner), m_m(n_m), m_scroll_x(n_scroll_x), m_scroll_y(n_scroll_y), m_selection(n_selection)
{
  assert(m_selection < m_m.num_links());
}


void menu_element::paint(graphics & g) const
{
  int i;

  for(i = 0; i < m_selection; ++i)
  {
    g.draw_bitmap(m_m.link_x(i), m_m.link_y(i), m_m.link_normal_picture(i));
  }
  g.draw_bitmap(m_m.link_x(m_selection), m_m.link_y(m_selection), m_m.link_selected_picture(m_selection));
  for(i = m_selection + 1; i < m_m.num_links(); ++i)
  {
    g.draw_bitmap(m_m.link_x(i), m_m.link_y(i), m_m.link_normal_picture(i));
  }
}


void menu_element::set_scroll_pos(int x, int y)
{
  m_scroll_x = x;
  m_scroll_y = y;
  if(visible())
  {
    m_owner.repaint(frame_x1(), frame_y1(), frame_x2(), frame_y2());
  }
}


void menu_element::set_selection(int n_selection)
{
  assert(n_selection < m_m.num_links());
  if(n_selection == m_selection) return;
  if(visible())
  {
    repaint(m_m.link_x(m_selection) - m_scroll_x, m_m.link_y(m_selection) - m_scroll_y, m_m.link_x(m_selection) + m_m.link_width(m_selection) - m_scroll_x, m_m.link_y(m_selection) + m_m.link_height(m_selection) - m_scroll_y);
    m_selection = n_selection;
    repaint(m_m.link_x(m_selection) - m_scroll_x, m_m.link_y(m_selection) - m_scroll_y, m_m.link_x(m_selection) + m_m.link_width(m_selection) - m_scroll_x, m_m.link_y(m_selection) + m_m.link_height(m_selection) - m_scroll_y);
  }
  else
  {
    m_selection = n_selection;
  }
}


