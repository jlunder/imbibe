#include "cplusplus.hh"

#include "bitmap_graphics.hh"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "bitmap.hh"
#include "color.hh"
#include "graphics.hh"
#include "pixel.hh"

#include "bitmap_graphics.ii"

#include "bitmap.ii"
#include "color.ii"
#include "graphics.ii"
#include "pixel.ii"


bitmap_graphics::bitmap_graphics(bitmap & n_b):
  m_b(n_b), m_clip_x1(0), m_clip_y1(0), m_clip_x2(n_b.width()), m_clip_y2(n_b.height()), m_bounds_x1(0), m_bounds_y1(0), m_bounds_x2(n_b.width()), m_bounds_y2(n_b.height())
{
}


int bitmap_graphics::width() const
{
  return m_b.width();
}


int bitmap_graphics::height() const
{
  return m_b.height();
}


int bitmap_graphics::bounds_x1() const
{
  return m_bounds_x1;
}


int bitmap_graphics::bounds_y1() const
{
  return m_bounds_y1;
}


int bitmap_graphics::bounds_x2() const
{
  return m_bounds_x2;
}


int bitmap_graphics::bounds_y2() const
{
  return m_bounds_y2;
}


int bitmap_graphics::bounds_width() const
{
  return m_bounds_x2 - m_bounds_x1;
}


int bitmap_graphics::bounds_height() const
{
  return m_bounds_y2 - m_bounds_y1;
}


int bitmap_graphics::clip_x1() const
{
  return m_clip_x1;
}


int bitmap_graphics::clip_y1() const
{
  return m_clip_y1;
}


int bitmap_graphics::clip_x2() const
{
  return m_clip_x2;
}


int bitmap_graphics::clip_y2() const
{
  return m_clip_y2;
}


void bitmap_graphics::set_bounds(int x1, int y1, int x2, int y2)
{
  m_bounds_x1 = x1;
  m_bounds_y1 = y1;
  m_bounds_x2 = x2;
  m_bounds_y2 = y2;
}


void bitmap_graphics::set_clip(int x1, int y1, int x2, int y2)
{
  m_clip_x1 = x1;
  m_clip_y1 = y1;
  m_clip_x2 = x2;
  m_clip_y2 = y2;
}


void bitmap_graphics::draw_rectangle(int x1, int y1, int x2, int y2, pixel p)
{
  int i;
  int j;
  int x = x1 + m_bounds_x1;
  int y = y1 + m_bounds_y1;
  int width = x2 - x1;
  int height = y2 - y1;

  if((y < m_clip_y2) && (y + height > m_clip_y1))
  {
    if((x < m_clip_x2) && (x + width > m_clip_x1))
    {
      if(x < m_clip_x1)
      {
        width -= m_clip_x1 - x;
        x = m_clip_x1;
      }
      if(y < m_clip_y1)
      {
        height -= m_clip_y1 - y;
        y = m_clip_y1;
      }
      if(x + width > m_clip_x2)
      {
        width = m_clip_x2 - x;
      }
      if(y + height > m_clip_y2)
      {
        height = m_clip_y2 - y;
      }
      for(i = 0; i < height; ++i)
      {
        for(j = 0; j < width; ++j)
        {
          m_b.at(x + j, y + i) = p;
        }
      }
    }
  }
}


void bitmap_graphics::draw_text(int x, int y, color c, char const * s)
{
  int i;
  size_t s_len = strlen(s);

  x += m_bounds_x1;
  y += m_bounds_y1;

  if((y < m_clip_y2) && (y >= m_clip_y1))
  {
    if((x < m_clip_x2) && (x + s_len > m_clip_x1))
    {
      if(x < m_clip_x1)
      {
        s += m_clip_x1 - x;
        s_len -= m_clip_x1 - x;
        x = m_clip_x1;
      }
      if(x + s_len > m_clip_x2)
      {
        s_len = m_clip_x2 - x;
      }
      for(i = 0; i < s_len; ++i)
      {
        m_b.at(x + i, y) = pixel(*s, c);
        ++s;
      }
    }
  }
}


void bitmap_graphics::draw_bitmap(int x, int y, bitmap const & b)
{
  int dest_x = x + m_bounds_x1;
  int dest_y = y + m_bounds_y1;
  int source_x1 = 0;
  int source_y1 = 0;
  int source_x2 = b.width();
  int source_y2 = b.height();

  if((dest_y < m_clip_y2) && (dest_y + b.height() > m_clip_y1))
  {
    if((dest_x < m_clip_x2) && (dest_x + b.width() > m_clip_x1))
    {
      if(dest_x < m_clip_x1)
      {
        source_x1 += m_clip_x1 - dest_x;
        dest_x = m_clip_x1;
      }
      if(dest_y < m_clip_y1)
      {
        source_y1 += m_clip_y1 - dest_y;
        dest_y = m_clip_y1;
      }
      if(dest_x + b.width() > m_clip_x2)
      {
        source_x2 -= dest_x + b.width() - m_clip_x2 - source_x1;
      }
      if(dest_y + b.height() > m_clip_y2)
      {
        source_y2 -= dest_y + b.height() - m_clip_y2 - source_y1;
      }
      if(&b != &m_b)
      {
        m_b.copy_bitmap(dest_x, dest_y, b, source_x1, source_y1, source_x2, source_y2);
      }
      else
      {
        m_b.copy_this_bitmap(dest_x, dest_y, source_x1, source_y1, source_x2, source_y2);
      }
    }
  }
}


