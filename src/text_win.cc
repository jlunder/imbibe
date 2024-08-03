#include "imbibe.hh"

#include "text_window.hh"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "bitmap.hh"
#include "element.hh"

#include "text_window.ii"

#include "bitmap.ii"
#include "element.ii"


text_window::text_window():
  m_backbuffer(new bitmap(80, 25)), m_locked(false)
{
  save_mode();
  set_text_mode();
}


text_window::~text_window()
{
  restore_mode();
}


void text_window::lock()
{
  ++m_locked;
}


void text_window::unlock()
{
  --m_locked;
  if(!m_locked)
  {
    if(m_need_repaint)
    {
      if(m_repaint_z_minus_infinity)
      {
        repaint(m_repaint_x1, m_repaint_y1, m_repaint_x2, m_repaint_y2);
      }
      else
      {
        repaint(m_repaint_x1, m_repaint_y1, m_repaint_x2, m_repaint_y2, m_repaint_z);
      }
      m_need_repaint = false;
    }
  }
}


void text_window::repaint()
{
  repaint(0, 0, m_backbuffer->width(), m_backbuffer->height());
}


void text_window::repaint(int x1, int y1, int x2, int y2)
{
  element_list_iterator i;

  if(!m_locked)
  {
    for(i = m_elements.begin(); i != m_elements.end(); ++i)
    {
      repaint_element(*i->ref, x1, y1, x2, y2);
    }
    flip(m_backbuffer);
  }
  else
  {
    locked_repaint(x1, y1, x2, y2);
  }
}


void text_window::repaint(int x1, int y1, int x2, int y2, int z)
{
  element_list_iterator i;

  if(!m_locked)
  {
    for(i = m_elements.lower_bound(z); i != m_elements.end(); ++i)
    {
      repaint_element(*i->ref, x1, y1, x2, y2);
    }
    flip(m_backbuffer);
  }
  else
  {
    locked_repaint(x1, y1, x2, y2, z);
  }
}


void text_window::add_element(element & e)
{
  m_elements.insert(element_list_value(e.frame_z(), &e));
  repaint(e.frame_x1(), e.frame_y1(), e.frame_x2(), e.frame_y2(), e.frame_z());
}


void text_window::remove_element(element & e)
{
  element_list_iterator i;

  for(i = m_elements.begin(); (i != m_elements.end()) && (i->ref != &e); ++i);
  m_elements.erase(i);

  repaint(e.frame_x1(), e.frame_y1(), e.frame_x2(), e.frame_y2());
}


void text_window::element_frame_pos_changed(element & e, int old_x1, int old_y1)
{
  int x1;
  int y1;
  int x2;
  int y2;

  if(old_x1 < e.frame_x1())
  {
    x1 = old_x1;
    x2 = e.frame_x2();
  }
  else
  {
    x1 = e.frame_x1();
    x2 = e.frame_x2() - e.frame_x1() + old_x1;
  }
  if(old_y1 < e.frame_y1())
  {
    y1 = old_y1;
    y2 = e.frame_y2();
  }
  else
  {
    y1 = e.frame_y1();
    y2 = e.frame_y2() - e.frame_y1() + old_y1;
  }
  repaint(x1, y1, x2, y2);
}


void text_window::element_frame_size_changed(element & e, int old_width, int old_height)
{
  int x1;
  int y1;
  int x2;
  int y2;

  x1 = e.frame_x1();
  y1 = e.frame_y1();
  if(old_width < e.frame_x2() - e.frame_x1()) x2 = e.frame_x2();
  else x2 = e.frame_x1() + old_width;
  if(old_height < e.frame_y2() - e.frame_y1()) y2 = e.frame_y2();
  else y2 = e.frame_y1() + old_height;
  repaint(x1, y1, x2, y2);
}


void text_window::element_frame_depth_changed(element & e, int old_z)
{
  element_list_iterator i;

  for(i = m_elements.begin(); (i != m_elements.end()) && (i->ref != &e); ++i);
  m_elements.erase(i);
  m_elements.insert(element_list_value(e.frame_z(), &e));
  repaint(e.frame_x1(), e.frame_y1(), e.frame_x2(), e.frame_y2(), e.frame_z());
}


void text_window::element_frame_changed(element & e, int old_x1, int old_y1, int old_x2, int old_y2, int old_z)
{
  element_list_iterator i;
  int x1;
  int y1;
  int x2;
  int y2;

  if(old_x1 < e.frame_x1()) x1 = old_x1;
  else x1 = e.frame_x1();
  if(old_y1 < e.frame_y1()) y1 = old_y1;
  else y1 = e.frame_y1();
  if(old_x2 > e.frame_x2()) x2 = old_x2;
  else x2 = e.frame_x2();
  if(old_y2 > e.frame_y2()) y2 = old_y2;
  else y2 = e.frame_y2();

  if(e.frame_z() != old_z)
  {
    for(i = m_elements.begin(); (i != m_elements.end()) && (i->ref != &e); ++i);
    m_elements.erase(i);
    m_elements.insert(element_list_value(e.frame_z(), &e));
  }
  repaint(x1, y1, x2, y2);
}


void text_window::repaint_element(element const & e, int x1, int y1, int x2, int y2)
{
  int t_x1;
  int t_y1;
  int t_x2;
  int t_y2;
  graphics & g = m_backbuffer->g();

  if((x1 < e.frame_x2()) && (x2 > e.frame_x1()) && (y1 < e.frame_y2()) && (y2 > e.frame_y1()))
  {
    if(x1 > e.frame_x1()) t_x1 = x1;
    else t_x1 = e.frame_x1();
    if(y1 > e.frame_y1()) t_y1 = y1;
    else t_y1 = e.frame_y1();
    if(x2 < e.frame_x2()) t_x2 = x2;
    else t_x2 = e.frame_x2();
    if(y2 < e.frame_y2()) t_y2 = y2;
    else t_y2 = e.frame_y2();

    if(t_x1 < 0) t_x1 = 0;
    if(t_y1 < 0) t_y1 = 0;
    if(t_x2 > m_backbuffer->width()) t_x2 = m_backbuffer->width();
    if(t_y2 > m_backbuffer->height()) t_y2 = m_backbuffer->height();

    g.set_bounds(e.frame_x1(), e.frame_y1(), e.frame_x2(), e.frame_y2());
    g.set_clip(t_x1, t_y1, t_x2, t_y2);
    e.paint(g);
  }
}


void text_window::locked_repaint(int x1, int y1, int x2, int y2)
{
  if(m_need_repaint)
  {
    if(x1 < m_repaint_x1) m_repaint_x1 = x1;
    if(y1 < m_repaint_y1) m_repaint_y1 = y1;
    if(x2 > m_repaint_x2) m_repaint_x2 = x2;
    if(y2 > m_repaint_y2) m_repaint_y2 = y2;
    m_repaint_z_minus_infinity = true;
  }
  else
  {
    m_need_repaint = true;
    m_repaint_x1 = x1;
    m_repaint_y1 = y1;
    m_repaint_x2 = x2;
    m_repaint_y2 = y2;
    m_repaint_z_minus_infinity = true;
  }
}


void text_window::locked_repaint(int x1, int y1, int x2, int y2, int z)
{
  if(m_need_repaint)
  {
    if(x1 < m_repaint_x1) m_repaint_x1 = x1;
    if(y1 < m_repaint_y1) m_repaint_y1 = y1;
    if(x2 > m_repaint_x2) m_repaint_x2 = x2;
    if(y2 > m_repaint_y2) m_repaint_y2 = y2;
    if(z < m_repaint_z) m_repaint_z = z;
  }
  else
  {
    m_need_repaint = true;
    m_repaint_x1 = x1;
    m_repaint_y1 = y1;
    m_repaint_x2 = x2;
    m_repaint_y2 = y2;
    m_repaint_z = z;
    m_repaint_z_minus_infinity = false;
  }
}


extern void set_text_asm();
#pragma aux set_text_asm = "mov ax, 00003h" "int 010h" modify exact [eax] nomemory


extern void restore_text_asm();
#pragma aux restore_text_asm = "mov ax, 00003h" "int 010h" modify exact [eax] nomemory


static void text_window::save_mode()
{
}


static void text_window::restore_mode()
{
  restore_text_asm();
}


static void text_window::set_text_mode()
{
  set_text_asm();
}


static void text_window::flip(bitmap * backbuffer)
{
  memcpy((unsigned short *)0xB8000, backbuffer->data(), backbuffer->width() * backbuffer->height() * sizeof(unsigned short));
}


