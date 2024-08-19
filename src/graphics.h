#ifndef __GRAPHICS_H_INCLUDED
#define __GRAPHICS_H_INCLUDED


#include "imbibe.h"


#include "bitmap.h"
#include "termviz.h"


namespace aux_graphics {
  struct clip_params;
  struct bitmap_transform_params;
}


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

  graphics(bitmap & n_b);
  virtual ~graphics() { }

  bool subregion_trivial() const
    { return (m_clip_x1 >= m_clip_x2) || (m_clip_y1 >= m_clip_y2); }

  void enter_subregion(subregion_state & save, coord_t x, coord_t y,
    coord_t clip_x1, coord_t clip_y1, coord_t clip_x2, coord_t clip_y2);
  void leave_subregion(subregion_state const & restore);
  void draw_rectangle(coord_t x1, coord_t y1, coord_t x2, coord_t y2,
    termel_t p);
  void draw_text(coord_t x, coord_t y, attribute_t attr, char const * s);
  void draw_bitmap(coord_t x, coord_t y, bitmap const & b);
  void draw_bitmap_fade(coord_t x, coord_t y, bitmap const & b, uint8_t fade);

  bitmap & b() { return m_b; }
  bitmap const & b() const { return m_b; }

  coord_t origin_x() const { return m_x; }
  coord_t origin_y() const { return m_y; }
  coord_t clip_x1() const { return m_clip_x1; }
  coord_t clip_y1() const { return m_clip_y1; }
  coord_t clip_x2() const { return m_clip_x2; }
  coord_t clip_y2() const { return m_clip_y2; }

private:
  bitmap & m_b;

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


