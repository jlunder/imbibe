#ifndef __BITMAP_H_INCLUDED
#define __BITMAP_H_INCLUDED


#include "imbibe.h"


class bitmap;


#include "color.h"
#include "pixel.h"


class bitmap
{
public:
  bitmap(bitmap const & n_bitmap);
  bitmap(coord_t n_width, coord_t n_height);
  ~bitmap();
  coord_t width() const { return m_width; }
  coord_t height() const { return m_height; }
  uint16_t & at(coord_t x, coord_t y) { return m_data[y * m_width + x]; }
  uint16_t const & at(coord_t x, coord_t y) const
    { return m_data[y * m_width + x]; }
  uint16_t * data() { return m_data; }
  uint16_t const * data() const { return m_data; }

protected:
  void copy_bitmap(coord_t dest_x, coord_t dest_y, bitmap const & source,
    coord_t source_x1, coord_t source_y1, coord_t source_x2,
    coord_t source_y2);

private:
  coord_t m_width;
  coord_t m_height;
  uint16_t * m_data;

  friend class bitmap_graphics;
};


#endif // __BITMAP_H_INCLUDED


