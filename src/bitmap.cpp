#include "imbibe.h"

#include "bitmap.h"

#include <string.h>

bitmap::bitmap(bitmap const &n_bitmap) : m_width(0), m_height(0), m_data(NULL) {
  assign(n_bitmap);
}

bitmap::bitmap(coord_t n_width, coord_t n_height, termel_t const __far *n_data)
    : m_width(0), m_height(0), m_data(NULL) {
  assign(n_width, n_height, n_data);
}

bitmap &bitmap::assign(coord_t n_width, coord_t n_height,
                       termel_t const __far *n_data) {
  assert(n_width >= 0);
  assert(n_height >= 0);
  assert_margin(n_width, COORD_MAX);
  assert_margin(n_height, COORD_MAX);
  m_width = n_width;
  m_height = n_height;
  m_data = const_cast<termel_t __far *>(n_data);

  return *this;
}

bitmap &bitmap::assign(bitmap const &n_bitmap) {
  m_width = n_bitmap.m_width;
  m_height = n_bitmap.m_height;
  assert_margin(m_width, COORD_MAX);
  assert_margin(m_height, COORD_MAX);
  m_data = n_bitmap.m_data;

  return *this;
}
