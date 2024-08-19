#include "imbibe.h"

#include "bitmap.h"

#include <string.h>


bitmap::bitmap(bitmap const & n_bitmap):
  m_width(n_bitmap.m_width), m_height(n_bitmap.m_height)
{
  assert_margin(m_width, COORD_MAX); assert_margin(m_height, COORD_MAX);
  m_data =
    (termel_t *)::malloc(m_width * m_height * sizeof (termel_t));
  assert(m_data);
  memcpy(m_data, n_bitmap.m_data, m_width * m_height * sizeof (termel_t));
}


bitmap::bitmap(coord_t n_width, coord_t n_height):
  m_width(n_width), m_height(n_height)
{
  assert(n_width >= 0); assert(n_height >= 0);
  assert_margin(n_width, COORD_MAX); assert_margin(n_height, COORD_MAX);
  assert((uint32_t)n_width * n_height + sizeof (termel_t)
    < (uint32_t)UINT16_MAX - 31);
  m_data =
    (termel_t *)::malloc(n_width * n_height * sizeof (termel_t));
  assert(m_data);
}


bitmap::~bitmap()
{
  ::free(m_data);
}


