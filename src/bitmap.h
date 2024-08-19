#ifndef __BITMAP_H_INCLUDED
#define __BITMAP_H_INCLUDED


#include "imbibe.h"

#include "termviz.h"


class bitmap
{
public:
  bitmap(bitmap const & n_bitmap);
  bitmap(coord_t n_width, coord_t n_height);
  ~bitmap();
  coord_t width() const { return m_width; }
  coord_t height() const { return m_height; }
  termel_t & at(coord_t x, coord_t y) { return m_data[y * m_width + x]; }
  termel_t const & at(coord_t x, coord_t y) const
    { return m_data[y * m_width + x]; }
  termel_t * data() { return m_data; }
  termel_t const * data() const { return m_data; }

private:
  coord_t m_width;
  coord_t m_height;
  termel_t * m_data;
};


#endif // __BITMAP_H_INCLUDED


