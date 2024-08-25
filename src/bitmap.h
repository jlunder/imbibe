#ifndef __BITMAP_H_INCLUDED
#define __BITMAP_H_INCLUDED


#include "imbibe.h"

#include "termviz.h"


class bitmap
{
public:
  bitmap(): m_width(0), m_height(0x8000), m_data(NULL) { }
  bitmap(bitmap const & n_bitmap);
  bitmap(coord_t n_width, coord_t n_height);
  // In the case of preallocated data, the data is treated as immutable and
  // will not be freed!
  bitmap(coord_t n_width, coord_t n_height, termel_t const * n_data);
  ~bitmap();

  coord_t width() const { return m_width; }
  coord_t height() const { return m_height & 0x7FFF; }
  bool immutable() const { return (m_height & 0x8000) != 0; }
  termel_t at(coord_t x, coord_t y) const {
    assert(x >= 0); assert(x < width());
    assert(y >= 0); assert(y < height());
    return m_data[y * m_width + x];
  }
  termel_t * data() { assert(!immutable()); return m_data; }
  termel_t const * data() const { return m_data; }

  void set_at(coord_t x, coord_t y, termel_t n_t) {
    assert(!immutable());
    assert(x >= 0); assert(x < width());
    assert(y >= 0); assert(y < height());
    m_data[y * m_width + x] = n_t;
  }

  bitmap & assign(coord_t n_width, coord_t n_height, termel_t const * n_data);
  bitmap & assign(bitmap const & n_bitmap);

  bitmap & operator = (bitmap const & other) { return assign(other); }

private:
  coord_t m_width;
  coord_t m_height;
  termel_t * m_data;
};


#endif // __BITMAP_H_INCLUDED


