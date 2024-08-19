#include "imbibe.h"

// #include "hbin_menu_handler.h"
#include "hbin_men.h"

#include "key_manager.h"
#include "hbin.h"
// #include "hbin_element.h"
#include "hbin_ele.h"


hbin_menu_handler::hbin_menu_handler(hbin & n_hb, hbin_element & n_hbe):
  m_hb(n_hb), m_hbe(n_hbe)
{
}


bool hbin_menu_handler::handle(int c)
{
  int x;
  int y;
  int selection;

  switch(c)
  {
  case key_event::up:
    selection = (m_hbe.selection() + m_hb.num_links() - 1) % m_hb.num_links();
    break;
  case key_event::down:
    selection = (m_hbe.selection() + m_hb.num_links() + 1) % m_hb.num_links();
    break;
  default:
    return false;
  }
  x = m_hbe.scroll_x();
  y = m_hbe.scroll_y();
  if(x + m_hbe.frame_width() < m_hb.link_x(selection) + m_hb.link_picture(selection).width())
  {
    x = m_hb.link_x(selection) + m_hb.link_picture(selection).width() - m_hbe.frame_width();
  }
  if(x > m_hb.link_x(selection))
  {
    x = m_hb.link_x(selection);
  }
  if(y + m_hbe.frame_height() < m_hb.link_y(selection) + m_hb.link_picture(selection).height())
  {
    y = m_hb.link_y(selection) + m_hb.link_picture(selection).height() - m_hbe.frame_height();
  }
  if(y > m_hb.link_y(selection))
  {
    y = m_hb.link_y(selection);
  }
  if((x != m_hbe.scroll_x()) || (y != m_hbe.scroll_y()))
  {
    m_hbe.owner().lock_repaint();
    m_hbe.set_scroll_pos(x, y);
    m_hbe.set_selection(selection);
    m_hbe.owner().unlock_repaint();
  }
  else
  {
    m_hbe.set_selection(selection);
  }
  return true;
}


