#ifndef __BITMAP_H_INCLUDED
#define __BITMAP_H_INCLUDED

#include "imbibe.h"

#include "termviz.h"

class bitmap {
public:
  bitmap() : m_width(0), m_height(0), m_data(NULL) {}
  bitmap(bitmap const &n_bitmap);
  // In the case of preallocated data, the data is treated as immutable and
  // will not be freed!
  bitmap(coord_t n_width, coord_t n_height, termel_t const __far *n_data);

  coord_t width() const { return m_width; }
  coord_t height() const { return m_height; }
  termel_t at(coord_t x, coord_t y) const {
    assert(x >= 0);
    assert(x < width());
    assert(y >= 0);
    assert(y < height());
    return m_data[y * m_width + x];
  }
  termel_t __far *data() { return m_data; }
  termel_t const __far *data() const { return m_data; }

  void set_at(coord_t x, coord_t y, termel_t n_t) {
    assert(x >= 0);
    assert(x < width());
    assert(y >= 0);
    assert(y < height());
    m_data[y * m_width + x] = n_t;
  }

  bitmap &assign(coord_t n_width, coord_t n_height,
                 termel_t const __far *n_data);
  bitmap &assign(bitmap const &n_bitmap);

  bitmap &operator=(bitmap const &other) { return assign(other); }

private:
  coord_t m_width;
  coord_t m_height;
  termel_t __far *m_data;
};

#endif // __BITMAP_H_INCLUDED
