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

  virtual void draw_rectangle(coord_t x1, coord_t y1, coord_t x2, coord_t y2,
    pixel p);
  virtual void draw_text(coord_t x, coord_t y, color c, char const * s);
  virtual void draw_bitmap(coord_t x, coord_t y, bitmap const & b);

private:
  bitmap & m_b;
};


#endif // __BITMAP_GRAPHICS_H_INCLUDED


