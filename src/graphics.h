#ifndef __GRAPHICS_H_INCLUDED
#define __GRAPHICS_H_INCLUDED


#include "imbibe.h"


class graphics;


#include "bitmap.h"
#include "color.h"
#include "pixel.h"


class graphics {
public:
  graphics(int16_t n_x1, int16_t n_y1, int16_t n_x2, int16_t n_y2)
    : m_bounds_x1(n_x1), m_bounds_y1(n_y1), m_bounds_x2(n_x2),
      m_bounds_y2(n_y2), m_clip_x1(n_x1), m_clip_y1(n_y1),
      m_clip_x2(n_x2), m_clip_y2(n_y2) { }
  virtual ~graphics() { }
  int16_t bounds_x1() const { return m_bounds_x1; }
  int16_t bounds_y1() const { return m_bounds_y1; }
  int16_t bounds_x2() const { return m_bounds_x2; }
  int16_t bounds_y2() const { return m_bounds_y2; }
  int16_t bounds_width() const { return m_bounds_x2 - m_bounds_x1; }
  int16_t bounds_height() const { return m_bounds_y2 - m_bounds_y1; }

  // Setting bounds invalidates the clip rectangle, follow with set_clip()
  virtual void set_bounds(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
  virtual void set_clip(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
  virtual void draw_rectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
    pixel p) = 0;
  virtual void draw_text(int16_t x, int16_t y, color c, char const * s) = 0;
  virtual void draw_bitmap(int16_t x, int16_t y, bitmap const & b) = 0;

protected:
  int16_t clip_x1() const { return m_clip_x1; }
  int16_t clip_y1() const { return m_clip_y1; }
  int16_t clip_x2() const { return m_clip_x2; }
  int16_t clip_y2() const { return m_clip_y2; }

private:
  int16_t m_bounds_x1;
  int16_t m_bounds_y1;
  int16_t m_bounds_x2;
  int16_t m_bounds_y2;
  int16_t m_clip_x1;
  int16_t m_clip_y1;
  int16_t m_clip_x2;
  int16_t m_clip_y2;
};


#endif // __GRAPHICS_H_INCLUDED


