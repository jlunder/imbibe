#include "imbibe.h"

#include "bitmap.h"

#include "color.h"
#include "pixel.h"

#include <string.h>


bitmap::bitmap(bitmap const & n_bitmap):
  m_width(n_bitmap.m_width), m_height(n_bitmap.m_height)
{
  assert_margin(m_width, COORD_MAX); assert_margin(m_height, COORD_MAX);
  m_data = (uint16_t *)::malloc(m_width * m_height * sizeof (uint16_t));
  assert(m_data);
  memcpy(m_data, n_bitmap.m_data, m_width * m_height * sizeof (uint16_t));
}


bitmap::bitmap(coord_t n_width, coord_t n_height):
  m_width(n_width), m_height(n_height)
{
  assert(n_width >= 0); assert(n_height >= 0);
  assert_margin(n_width, COORD_MAX); assert_margin(n_height, COORD_MAX);
  assert((uint32_t)n_width * n_height + sizeof (uint16_t)
    < (uint32_t)UINT16_MAX - 31);
  m_data = (uint16_t *)::malloc(n_width * n_height * sizeof (uint16_t));
  assert(m_data);
}


bitmap::~bitmap()
{
  ::free(m_data);
}


void bitmap::copy_bitmap(coord_t dest_x, coord_t dest_y,
    bitmap const & source, coord_t source_x1, coord_t source_y1,
    coord_t source_x2, coord_t source_y2)
{
  assert(dest_x >= 0); assert(dest_y >= 0);
  assert(dest_x + (source_x2 - source_x1) <= m_width);
  assert(dest_y + (source_y2 - source_y1) <= m_height);
  assert(source_x1 >= 0); assert(source_y1 >= 0);
  assert(source_x2 <= source.m_width); assert(source_y2 <= source.m_height);
  assert(source_x1 <= source_x2); assert(source_y1 <= source_y2);

  coord_t i;
  coord_t bytes_per_line = (source_x2 - source_x1) * sizeof (uint16_t);
  coord_t lines = source_y2 - source_y1;
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


