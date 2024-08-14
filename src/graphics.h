#ifndef __GRAPHICS_H_INCLUDED
#define __GRAPHICS_H_INCLUDED


#include "imbibe.h"


class graphics;


#include "bitmap.h"
#include "color.h"
#include "pixel.h"


class graphics {
public:
  class subregion_state {
  private:
    coord_t m_x;
    coord_t m_y;
    coord_t m_clip_x1;
    coord_t m_clip_y1;
    coord_t m_clip_x2;
    coord_t m_clip_y2;
#ifndef NDEBUG
    coord_t m_subregion_depth;
#endif

    friend class graphics;
  };

  graphics(coord_t n_clip_x1, coord_t n_clip_y1, coord_t n_clip_x2,
      coord_t n_clip_y2);
  graphics();
  virtual ~graphics() { }

  bool subregion_trivial() const
    { return (m_clip_x1 >= m_clip_x2) || (m_clip_y1 >= m_clip_y2); }

  virtual void enter_subregion(subregion_state & save, coord_t x, coord_t y,
    coord_t clip_x1, coord_t clip_y1, coord_t clip_x2, coord_t clip_y2);
  virtual void leave_subregion(subregion_state const & restore);
  virtual void draw_rectangle(coord_t x1, coord_t y1, coord_t x2, coord_t y2,
    pixel p) = 0;
  virtual void draw_text(coord_t x, coord_t y, color c, char const * s) = 0;
  virtual void draw_bitmap(coord_t x, coord_t y, bitmap const & b) = 0;

  coord_t origin_x() const { return m_x; }
  coord_t origin_y() const { return m_y; }
  coord_t clip_x1() const { return m_clip_x1; }
  coord_t clip_y1() const { return m_clip_y1; }
  coord_t clip_x2() const { return m_clip_x2; }
  coord_t clip_y2() const { return m_clip_y2; }

private:
  coord_t m_x;
  coord_t m_y;
  coord_t m_clip_x1;
  coord_t m_clip_y1;
  coord_t m_clip_x2;
  coord_t m_clip_y2;

#ifndef NDEBUG
  coord_t m_subregion_depth;
#endif
};


#endif // __GRAPHICS_H_INCLUDED


