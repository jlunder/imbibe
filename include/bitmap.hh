#ifndef __BITMAP_HH_INCLUDED
#define __BITMAP_HH_INCLUDED


class bitmap;


#include "color.hh"
#include "graphics.hh"
#include "pixel.hh"


class bitmap
{
public:
  bitmap(bitmap const & n_bitmap);
  bitmap(int n_width, int n_height);
  ~bitmap();
  int width() const;
  int height() const;
  graphics & g();
  unsigned short & at(int x, int y);
  unsigned short const & at(int x, int y) const;
  unsigned short * data();
  unsigned short const * data() const;

protected:
  void copy_this_bitmap(int dest_x, int dest_y, int source_x1, int source_y1, int source_x2, int source_y2);
  void copy_bitmap(int x, int y, bitmap const & source);
  void copy_bitmap(int dest_x, int dest_y, bitmap const & source, int source_x1, int source_y1, int source_x2, int source_y2);

private:
  int m_width;
  int m_height;
  graphics * m_g;
  unsigned short * m_data;

  friend class bitmap_graphics;
};


#endif //__BITMAP_HH_INCLUDED


