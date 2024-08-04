#ifndef __GRAPHICS_HH_INCLUDED
#define __GRAPHICS_HH_INCLUDED


#include "imbibe.hh"


class graphics;


#include "bitmap.hh"
#include "color.hh"
#include "pixel.hh"


class graphics
{
public:
  virtual int width() const = 0;
  virtual int height() const = 0;
  virtual int bounds_x1() const = 0;
  virtual int bounds_y1() const = 0;
  virtual int bounds_x2() const = 0;
  virtual int bounds_y2() const = 0;
  virtual int bounds_width() const = 0;
  virtual int bounds_height() const = 0;
  virtual int clip_x1() const = 0;
  virtual int clip_y1() const = 0;
  virtual int clip_x2() const = 0;
  virtual int clip_y2() const = 0;
  virtual void set_bounds(int x1, int y1, int x2, int y2) = 0;
  virtual void set_clip(int x1, int y1, int x2, int y2) = 0;
  virtual void draw_rectangle(int x1, int y1, int x2, int y2, pixel p) = 0;
  virtual void draw_text(int x, int y, color c, char const * s) = 0;
  virtual void draw_bitmap(int x, int y, bitmap const & b) = 0;
};


#endif //__GRAPHICS_HH_INCLUDED


