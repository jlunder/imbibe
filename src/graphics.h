#ifndef __GRAPHICS_H_INCLUDED
#define __GRAPHICS_H_INCLUDED


#include "imbibe.h"


class graphics;


#include "bitmap.h"
#include "color.h"
#include "pixel.h"


class graphics {
public:
  virtual int16_t width() const = 0;
  virtual int16_t height() const = 0;
  virtual int16_t bounds_x1() const = 0;
  virtual int16_t bounds_y1() const = 0;
  virtual int16_t bounds_x2() const = 0;
  virtual int16_t bounds_y2() const = 0;
  virtual int16_t bounds_width() const = 0;
  virtual int16_t bounds_height() const = 0;
  virtual int16_t clip_x1() const = 0;
  virtual int16_t clip_y1() const = 0;
  virtual int16_t clip_x2() const = 0;
  virtual int16_t clip_y2() const = 0;
  virtual void set_bounds(int16_t x1, int16_t y1, int16_t x2, int16_t y2) = 0;
  virtual void set_clip(int16_t x1, int16_t y1, int16_t x2, int16_t y2) = 0;
  virtual void draw_rectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
    pixel p) = 0;
  virtual void draw_text(int16_t x, int16_t y, color c, char const * s) = 0;
  virtual void draw_bitmap(int16_t x, int16_t y, bitmap const & b) = 0;
};


#endif // __GRAPHICS_H_INCLUDED


