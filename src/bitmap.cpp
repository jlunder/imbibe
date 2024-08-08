#include "imbibe.h"

#include "bitmap.h"

#include "color.h"
#include "pixel.h"

#include <string.h>


bitmap::bitmap(bitmap const & n_bitmap):
  m_width(n_bitmap.m_width), m_height(n_bitmap.m_height)
{
  assert_margin(m_width, INT16_MAX); assert_margin(m_height, INT16_MAX);
  m_data = (uint16_t *)::malloc(m_width * m_height * sizeof (uint16_t));
  assert(m_data);
  memcpy(m_data, n_bitmap.m_data, m_width * m_height * sizeof (uint16_t));
}


bitmap::bitmap(int16_t n_width, int16_t n_height):
  m_width(n_width), m_height(n_height)
{
  assert(n_width >= 0); assert(n_height >= 0);
  assert_margin(n_width, INT16_MAX); assert_margin(n_height, INT16_MAX);
  assert_margin((uint32_t)n_width * (uint32_t)n_height,
    (uint32_t)UINT16_MAX / sizeof (uint16_t));
  m_data = (uint16_t *)::malloc(n_width * n_height * sizeof (uint16_t));
  assert(m_data);
}


bitmap::~bitmap()
{
  ::free(m_data);
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
  assert(dest_x >= 0); assert(dest_y >= 0);
  assert(dest_x + (source_x2 - source_x1) <= m_width);
  assert(dest_y + (source_y2 - source_y1) <= m_height);
  assert(source_x1 >= 0); assert(source_y1 >= 0);
  assert(source_x2 <= source.m_width); assert(source_y2 <= source.m_height);
  assert(source_x1 <= source_x2); assert(source_y1 <= source_y2);

  int16_t i;
  int16_t bytes_per_line = (source_x2 - source_x1) * sizeof (uint16_t);
  int16_t lines = source_y2 - source_y1;
  uint8_t const * source_p = (uint8_t const *)(
    source.m_data + source_y1 * source.m_width + source_x1);
  uint8_t * dest_p = (uint8_t *)(m_data + dest_y * m_width + dest_x);
  uint16_t source_stride = source.m_width * sizeof (uint16_t);
  uint16_t dest_stride = m_width * sizeof (uint16_t);

  for(i = 0; i < lines; ++i) {
    memcpy(dest_p, source_p, bytes_per_line);
    dest_p += dest_stride;
    source_p += source_stride;
  }
}


