#ifndef __BITMAP_GRAPHICS_HH_INCLUDED
#define __BITMAP_GRAPHICS_HH_INCLUDED


#include "imbibe.hh"


class bitmap_graphics;


#include "bitmap.hh"
#include "color.hh"
#include "graphics.hh"


class bitmap_graphics: public graphics
{
public:
  bitmap_graphics(bitmap & n_b);
  virtual int width() const;
  virtual int height() const;
  virtual int bounds_x1() const;
  virtual int bounds_y1() const;
  virtual int bounds_x2() const;
  virtual int bounds_y2() const;
  virtual int bounds_width() const;
  virtual int bounds_height() const;
  virtual int clip_x1() const;
  virtual int clip_y1() const;
  virtual int clip_x2() const;
  virtual int clip_y2() const;
  virtual void set_bounds(int x1, int y1, int x2, int y2);
  virtual void set_clip(int x1, int y1, int x2, int y2);
  virtual void draw_rectangle(int x1, int y1, int x2, int y2, pixel p);
  virtual void draw_text(int x, int y, color c, char const * s);
  virtual void draw_bitmap(int x, int y, bitmap const & b);

private:
  bitmap & m_b;
  int m_clip_x1;
  int m_clip_y1;
  int m_clip_x2;
  int m_clip_y2;
  int m_bounds_x1;
  int m_bounds_y1;
  int m_bounds_x2;
  int m_bounds_y2;
};


//#include "bitmap_graphics.ii"


#endif //__BITMAP_GRAPHICS_HH_INCLUDED


