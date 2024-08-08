#ifndef __BITMAP_GRAPHICS_H_INCLUDED
#define __BITMAP_GRAPHICS_H_INCLUDED


#include "imbibe.h"


class bitmap_graphics;


#include "bitmap.h"
#include "color.h"
#include "graphics.h"


class bitmap_graphics: public graphics
{
public:
  bitmap_graphics(bitmap & n_b);

  virtual void set_clip(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
  virtual void draw_rectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
    pixel p);
  virtual void draw_text(int16_t x, int16_t y, color c, char const * s);
  virtual void draw_bitmap(int16_t x, int16_t y, bitmap const & b);

private:
  bitmap & m_b;
};


#endif // __BITMAP_GRAPHICS_H_INCLUDED


