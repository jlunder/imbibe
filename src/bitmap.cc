#include "cplusplus.hh"

#include "bitmap.hh"

#include "color.hh"
#include "pixel.hh"

#include <string.h>

#include "bitmap.ii"

#include "color.ii"
#include "pixel.ii"


bitmap::bitmap(bitmap const & n_bitmap):
  m_width(n_bitmap.m_width), m_height(n_bitmap.m_height)
{
  m_data = new unsigned short[m_width * m_height];
  memcpy(m_data, n_bitmap.m_data, m_width * m_height * sizeof(unsigned short));
}


bitmap::bitmap(int n_width, int n_height):
  m_width(n_width), m_height(n_height)
{
  m_data = new unsigned short[n_width * n_height];
}


bitmap::~bitmap()
{
  delete m_g;
}


void bitmap::copy_this_bitmap(int dest_x, int dest_y, int source_x1, int source_y1, int source_x2, int source_y2)
{
  int i;
  int source_width = source_x2 - source_x1;
  int source_height = source_y2 - source_y1;

  if(dest_y < source_y1)
  {
    for(i = 0; i < source_height; ++i)
    {
      memmove(m_data + (i + dest_y) * m_width + dest_x, m_data + (i + source_y1) * m_width + source_x1, source_width * sizeof(unsigned short));
    }
  }
  else
  {
    for(i = source_height; i > 0; --i)
    {
      memmove(m_data + (i - 1 + dest_y) * m_width + dest_x, m_data + (i - 1 + source_y1) * m_width + source_x1, source_width * sizeof(unsigned short));
    }
  }
}


void bitmap::copy_bitmap(int x, int y, bitmap const & source)
{
  int i;

  for(i = 0; i < source.m_height; ++i)
  {
    memcpy(m_data + (i + y) * m_width + x, source.m_data + i * source.m_width, source.m_width * sizeof(unsigned short));
  }
}


void bitmap::copy_bitmap(int dest_x, int dest_y, bitmap const & source, int source_x1, int source_y1, int source_x2, int source_y2)
{
  int i;
  int source_width = source_x2 - source_x1;
  int source_height = source_y2 - source_y1;

  for(i = 0; i < source_height; ++i)
  {
    memcpy(m_data + (i + dest_y) * m_width + dest_x, source.m_data + (i + source_y1) * source.m_width + source_x1, source_width * sizeof(unsigned short));
  }
}


