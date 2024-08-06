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
  virtual int16_t width() const;
  virtual int16_t height() const;
  virtual int16_t bounds_x1() const;
  virtual int16_t bounds_y1() const;
  virtual int16_t bounds_x2() const;
  virtual int16_t bounds_y2() const;
  virtual int16_t bounds_width() const;
  virtual int16_t bounds_height() const;
  virtual int16_t clip_x1() const;
  virtual int16_t clip_y1() const;
  virtual int16_t clip_x2() const;
  virtual int16_t clip_y2() const;
  virtual void set_bounds(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
  virtual void set_clip(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
  virtual void draw_rectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
    pixel p);
  virtual void draw_text(int16_t x, int16_t y, color c, char const * s);
  virtual void draw_bitmap(int16_t x, int16_t y, bitmap const & b);

private:
  bitmap & m_b;
  int16_t m_clip_x1;
  int16_t m_clip_y1;
  int16_t m_clip_x2;
  int16_t m_clip_y2;
  int16_t m_bounds_x1;
  int16_t m_bounds_y1;
  int16_t m_bounds_x2;
  int16_t m_bounds_y2;
};


#endif // __BITMAP_GRAPHICS_H_INCLUDED


