#include "imbibe.h"

#include "bitmap.h"

#include <string.h>

bitmap::bitmap(bitmap const &n_bitmap)
    : m_width(0), m_height(0x8000), m_data(NULL) {
  assign(n_bitmap);
}

bitmap::bitmap(coord_t n_width, coord_t n_height)
    : m_width(0), m_height(0x8000), m_data(NULL) {
  assign(n_width, n_height);
}

bitmap::bitmap(coord_t n_width, coord_t n_height, termel_t const __far *n_data)
    : m_width(0), m_height(0x8000), m_data(NULL) {
  assign(n_width, n_height, n_data);
}

bitmap::~bitmap() {
  if (!immutable()) {
    _ffree(m_data);
  }
}

bitmap &bitmap::assign(coord_t n_width, coord_t n_height) {
  if (!immutable()) {
    _ffree(m_data);
  }

  assert(n_width >= 0);
  assert(n_height >= 0);
  assert_margin(n_width, COORD_MAX);
  assert_margin(n_height, COORD_MAX);
  assert((uint32_t)n_width * n_height + sizeof(termel_t) <
         (uint32_t)UINT16_MAX - 31);
  m_width = n_width;
  m_height = n_height;
  m_data = reinterpret_cast<termel_t __far *>(
      _fmalloc(n_width * n_height * sizeof(termel_t)));
  assert(m_data);
  assert(!immutable());

  return *this;
}

bitmap &bitmap::assign(coord_t n_width, coord_t n_height, termel_t fill_brush) {
  assign(n_width, n_height);
  segsize_t size = n_width * n_height;
  for (segsize_t i = 0; i < size; ++i) {
    m_data[i] = fill_brush;
  }

  return *this;
}

bitmap &bitmap::assign(coord_t n_width, coord_t n_height,
                       termel_t const __far *n_data) {
  if (!immutable()) {
    _ffree(m_data);
  }

  assert(n_width >= 0);
  assert(n_height >= 0);
  assert_margin(n_width, COORD_MAX);
  assert_margin(n_height, COORD_MAX);
  m_width = n_width;
  m_height = n_height | 0x8000;
  m_data = const_cast<termel_t __far *>(n_data);

  return *this;
}

bitmap &bitmap::assign(bitmap const &n_bitmap) {
  if (!immutable()) {
    _ffree(m_data);
  }

  m_width = n_bitmap.m_width;
  m_height = n_bitmap.m_height;
  assert_margin(m_width, COORD_MAX);
  assert_margin(m_height & 0x7FFF, COORD_MAX);
  if (n_bitmap.immutable()) {
    m_data = n_bitmap.m_data;
    assert(immutable());
  } else {
    m_data = reinterpret_cast<termel_t __far *>(
        _fmalloc(m_width * m_height * sizeof(termel_t)));
    assert(m_data);
    _fmemcpy(m_data, n_bitmap.m_data, m_width * m_height * sizeof(termel_t));
    assert(!immutable());
  }

  return *this;
}
