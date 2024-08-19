#include "imbibe.h"

// #include "hbin_view_handler.h"
#include "hbin_vie.h"

#include "key_manager.h"
#include "hbin.h"
// #include "hbin_element.h"
#include "hbin_ele.h"


hbin_view_handler::hbin_view_handler(hbin & n_hb, hbin_element & n_hbe):
  m_hb(n_hb), m_hbe(n_hbe)
{
}


void hbin_view_handler::select_link(int selection)
{
  int x;
  int y;

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
}


void hbin_view_handler::scroll_to(int x, int y, bool down)
{
  int i;

  if(x < 0)
  {
    x = 0;
  }
  if(x + m_hbe.frame_width() > m_hb.width())
  {
    x = m_hb.width() - m_hbe.frame_width();
  }
  if(y < 0)
  {
    y = 0;
  }
  if(y + m_hbe.frame_height() > m_hb.height())
  {
    y = m_hb.height() - m_hbe.frame_height();
  }

  if(m_hb.num_links() != 0)
  {
    if(down)
    {
      for(i = m_hbe.selection(); (i + 1) % m_hb.num_links() != m_hbe.selection(); i = (i + 1) % m_hb.num_links())
      {
        if(!((m_hb.link_x(i) >= x + m_hbe.frame_width())
             || (m_hb.link_x(i) + m_hb.link_width(i) <= x)
             || (m_hb.link_y(i) >= y + m_hbe.frame_height())
             || (m_hb.link_y(i) + m_hb.link_height(i) <= y)))
        {
          break;
        }
      }
    }
    else
    {
      for(i = m_hbe.selection(); (i + m_hb.num_links() - 1) % m_hb.num_links() != m_hbe.selection(); i = (i + m_hb.num_links() - 1) % m_hb.num_links())
      {
        if(!((m_hb.link_x(i) >= x + m_hbe.frame_width())
             || (m_hb.link_x(i) + m_hb.link_width(i) <= x)
             || (m_hb.link_y(i) >= y + m_hbe.frame_height())
             || (m_hb.link_y(i) + m_hb.link_height(i) <= y)))
        {
          break;
        }
      }
    }
    if(i != m_hbe.selection())
    {
      if((x != m_hbe.scroll_x()) || (y != m_hbe.scroll_y()))
      {
        m_hbe.owner().lock_repaint();
        m_hbe.set_scroll_pos(x, y);
        m_hbe.set_selection(i);
        m_hbe.owner().unlock_repaint();
      }
      else
      {
        m_hbe.set_selection(i);
      }
    }
    else
    {
      if((x != m_hbe.scroll_x()) || (y != m_hbe.scroll_y()))
      {
        m_hbe.set_scroll_pos(x, y);
      }
    }
  }
  else
  {
    if((x != m_hbe.scroll_x()) || (y != m_hbe.scroll_y()))
    {
      m_hbe.set_scroll_pos(x, y);
    }
  }
}


bool hbin_view_handler::handle_key(int c)
{
  switch(c)
  {
  case key_event::tab:
    if(m_hb.num_links() != 0) {
      select_link((m_hbe.selection() + 1) % m_hb.num_links());
    }
    break;
  case key_event::shift_tab:
    if(m_hb.num_links() != 0) {
      select_link((m_hbe.selection() + m_hb.num_links() - 1) % m_hb.num_links());
    }
    break;
  case key_event::up:
    scroll_to(m_hbe.scroll_x(), m_hbe.scroll_y() - 1, false);
    break;
  case key_event::down:
    scroll_to(m_hbe.scroll_x(), m_hbe.scroll_y() + 1, true);
    break;
  case key_event::left:
    scroll_to(m_hbe.scroll_x() - 1, m_hbe.scroll_y(), false);
    break;
  case key_event::right:
    scroll_to(m_hbe.scroll_x() + 1, m_hbe.scroll_y(), true);
    break;
  case key_event::pgup:
    scroll_to(m_hbe.scroll_x(), m_hbe.scroll_y() - m_hbe.frame_height(), false);
    break;
  case key_event::pgdown:
    scroll_to(m_hbe.scroll_x(), m_hbe.scroll_y() + m_hbe.frame_height(), true);
    break;
  case key_event::home:
    scroll_to(0, m_hbe.scroll_y(), true);
    break;
  case key_event::end:
    scroll_to(m_hb.width() - m_hbe.frame_width(), m_hbe.scroll_y(), false);
    break;
  default:
    return false;
  }
  return true;
}


