#include "imbibe.hh"

#include "menu_handler.hh"

#include "key_handler.hh"
#include "menu.hh"
#include "menu_element.hh"


menu_handler::menu_handler(menu & n_m, menu_element & n_me):
  m_m(n_m), m_me(n_me)
{
}


bool menu_handler::handle(int c)
{
  int x;
  int y;
  int selection;

  switch(c)
  {
  case key_up:
    selection = (m_me.selection() + m_m.num_links() - 1) % m_m.num_links();
    break;
  case key_down:
    selection = (m_me.selection() + m_m.num_links() + 1) % m_m.num_links();
    break;
  default:
    return false;
    break;
  }
  x = m_me.scroll_x();
  y = m_me.scroll_y();
  if(x + m_me.frame_width() < m_m.link_x(selection) + m_m.link_normal_picture(selection).width())
  {
    x = m_m.link_x(selection) + m_m.link_normal_picture(selection).width() - m_me.frame_width();
  }
  if(x > m_m.link_x(selection))
  {
    x = m_m.link_x(selection);
  }
  if(y + m_me.frame_height() < m_m.link_y(selection) + m_m.link_normal_picture(selection).height())
  {
    y = m_m.link_y(selection) + m_m.link_normal_picture(selection).height() - m_me.frame_height();
  }
  if(y > m_m.link_y(selection))
  {
    y = m_m.link_y(selection);
  }
  if((x != m_me.scroll_x()) || (y != m_me.scroll_y()))
  {
    m_me.owner().lock();
    m_me.set_scroll_pos(x, y);
    m_me.set_selection(selection);
    m_me.owner().unlock();
  }
  else
  {
    m_me.set_selection(selection);
  }
  return true;
}


