#ifndef __BITMAP_HH_INCLUDED
#define __BITMAP_HH_INCLUDED


#include "imbibe.h"


class bitmap;


#include "color.h"
#include "pixel.h"


class bitmap
{
public:
  bitmap(bitmap const & n_bitmap);
  bitmap(int16_t n_width, int16_t n_height);
  ~bitmap();
  int16_t width() const { return m_width; }
  int16_t height() const { return m_height; }
  uint16_t & at(int16_t x, int16_t y) { return m_data[y * m_width + x]; }
  int16_t const & at(int16_t x, int16_t y) const
    { return m_data[y * m_width + x]; }
  uint16_t * data() { return m_data; }
  uint16_t const * data() const { return m_data; }

protected:
  void copy_this_bitmap(int16_t dest_x, int16_t dest_y, int16_t source_x1,
    int16_t source_y1, int16_t source_x2, int16_t source_y2);
  void copy_bitmap(int16_t x, int16_t y, bitmap const & source);
  void copy_bitmap(int16_t dest_x, int16_t dest_y, bitmap const & source,
    int16_t source_x1, int16_t source_y1, int16_t source_x2,
    int16_t source_y2);

private:
  int16_t m_width;
  int16_t m_height;
  uint16_t * m_data;

  friend class bitmap_graphics;
};


#endif //__BITMAP_HH_INCLUDED


