#include "imbibe.hh"

#include "element.hh"

#include "window.hh"

#include "element.ii"

#include "window.ii"


element::element(element const & n_element):
  m_x1(n_element.m_x1), m_y1(n_element.m_y1), m_x2(n_element.m_x2), m_y2(n_element.m_y2), m_z(n_element.m_z), m_owner(n_element.m_owner)
{
}


element::element(int n_x1, int n_y1, int n_x2, int n_y2, int n_z, window & n_owner, bool n_visible):
  m_x1(n_x1), m_y1(n_y1), m_x2(n_x2), m_y2(n_y2), m_z(n_z), m_owner(n_owner), m_visible(n_visible)
{
}


void element::set_frame_pos(int x1, int y1)
{
  int old_x1 = m_x1;
  int old_y1 = m_y1;

  m_x2 = x1 + frame_width();
  m_y2 = y1 + frame_height();
  m_x1 = x1;
  m_y1 = y1;
  if(m_visible)
  {
    m_owner.element_frame_pos_changed(*this, old_x1, old_y1);
  }
}


void element::set_frame_size(int width, int height)
{
  int old_width = frame_width();
  int old_height = frame_height();

  m_x2 = m_x1 + width;
  m_y2 = m_y1 + height;
  if(m_visible)
  {
    m_owner.element_frame_size_changed(*this, old_width, old_height);
  }
}


void element::set_frame_depth(int z)
{
  int old_z = m_z;

  m_z = z;
  if(m_visible)
  {
    m_owner.element_frame_depth_changed(*this, old_z);
  }
}


void element::set_frame(int x1, int y1, int x2, int y2, int z)
{
  int old_x1 = m_x1;
  int old_y1 = m_y1;
  int old_x2 = m_x2;
  int old_y2 = m_y2;
  int old_z = z;

  m_x1 = x1;
  m_y1 = y1;
  m_x2 = x2;
  m_y2 = y2;
  if(m_visible)
  {
    m_owner.element_frame_changed(*this, old_x1, old_y1, old_x2, old_y2, z);
  }
}


void element::repaint()
{
  repaint(0, 0, frame_width(), frame_height());
}


void element::repaint(int x1, int y1, int x2, int y2)
{
  if(m_visible)
  {
    m_owner.repaint(x1+m_x1, y1+m_y1, x2+m_x1, y2+m_y1, m_z);
  }
}


