#include "imbibe.h"

#include "bitmap.h"

#include "color.h"
#include "pixel.h"

#include <string.h>


bitmap::bitmap(bitmap const & n_bitmap):
  m_width(n_bitmap.m_width), m_height(n_bitmap.m_height)
{
  m_data = new uint16_t[m_width * m_height];
  memcpy(m_data, n_bitmap.m_data, m_width * m_height * sizeof(uint16_t));
}


bitmap::bitmap(int16_t n_width, int16_t n_height):
  m_width(n_width), m_height(n_height)
{
  m_data = new uint16_t[n_width * n_height];
}


bitmap::~bitmap()
{
  delete[] m_data;
}


void bitmap::copy_this_bitmap(int16_t dest_x, int16_t dest_y,
    int16_t source_x1, int16_t source_y1, int16_t source_x2,
    int16_t source_y2) {
  int16_t i;
  int16_t source_width = source_x2 - source_x1;
  int16_t source_height = source_y2 - source_y1;

  if(dest_y < source_y1) {
    for(i = 0; i < source_height; ++i) {
      memmove(m_data + (i + dest_y) * m_width + dest_x,
        m_data + (i + source_y1) * m_width + source_x1,
        source_width * sizeof(uint16_t));
    }
  } else {
    for(i = source_height; i > 0; --i) {
      memmove(m_data + (i - 1 + dest_y) * m_width + dest_x,
        m_data + (i - 1 + source_y1) * m_width + source_x1,
        source_width * sizeof(uint16_t));
    }
  }
}


void bitmap::copy_bitmap(int16_t x, int16_t y, bitmap const & source)
{
  int16_t i;

  for(i = 0; i < source.m_height; ++i) {
    memcpy(m_data + (i + y) * m_width + x,
      source.m_data + i * source.m_width, source.m_width * sizeof(uint16_t));
  }
}


void bitmap::copy_bitmap(int16_t dest_x, int16_t dest_y,
    bitmap const & source, int16_t source_x1, int16_t source_y1,
    int16_t source_x2, int16_t source_y2)
{
  int16_t i;
  int16_t source_width = source_x2 - source_x1;
  int16_t source_height = source_y2 - source_y1;

  for(i = 0; i < source_height; ++i) {
    memcpy(m_data + (i + dest_y) * m_width + dest_x,
      source.m_data + (i + source_y1) * source.m_width + source_x1,
      source_width * sizeof(uint16_t));
  }
}


