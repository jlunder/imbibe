#ifndef __GRAPHICS_H_INCLUDED
#define __GRAPHICS_H_INCLUDED

#include "imbibe.h"

#include "imstring.h"

class bitmap;
class tbm;
class unpacker;

class graphics {
public:
  class subregion_state {
  private:
    point m_origin;
    rect m_clip;
#if BUILD_DEBUG
    coord_t m_subregion_depth;
#endif

    friend class graphics;
  };

  graphics(bitmap *n_b);
  virtual ~graphics() {}

  bool subregion_trivial() const { return m_clip.trivial(); }

  void enter_subregion(point sub_o, rect const &sub_clip,
                       subregion_state *out_save);
  void leave_subregion(subregion_state const *restore);
  void draw_rectangle(coord_t x1, coord_t y1, coord_t x2, coord_t y2,
                      termel_t p) {
    draw_rectangle(rect(x1, y1, x2, y2), p);
  }
  void blend_rectangle(coord_t x1, coord_t y1, coord_t x2, coord_t y2,
                       termel_t p, uint8_t fade) {
    blend_rectangle(rect(x1, y1, x2, y2), p, fade);
  }
  void draw_rectangle(rect const &r, termel_t p);
  void blend_rectangle(rect const &r, termel_t p, uint8_t fade);
#if BUILD_FAR_DATA
  void draw_text(coord_t x, coord_t y, attribute_t attr, char const *s);
#else
  void draw_text(coord_t x, coord_t y, attribute_t attr, char const __far *s);
  void draw_text(coord_t x, coord_t y, attribute_t attr, char const *s) {
    draw_text(x, y, attr, static_cast<char const __far *>(s));
  }
#endif
  void draw_text(coord_t x, coord_t y, attribute_t attr, imstring const &s) {
    draw_text(x, y, attr, s.c_str());
  }
  void draw_bitmap(coord_t x, coord_t y, bitmap const &b);
  void draw_bitmap_fade(coord_t x, coord_t y, bitmap const &b, uint8_t fade);
  void draw_tbm(coord_t x, coord_t y, tbm const &tbm_data);
  void draw_tbm_fade(coord_t x, coord_t y, tbm const &tbm_data, uint8_t fade);

  bitmap *b() { return m_b; }
  bitmap const *b() const { return m_b; }

  point const &origin() const { return m_origin; }
  rect clip() const { return m_clip; }

private:
  bitmap *m_b;

  point m_origin;
  rect m_clip;

#if BUILD_DEBUG
  coord_t m_subregion_depth;
#endif
};

#endif // __GRAPHICS_H_INCLUDED
