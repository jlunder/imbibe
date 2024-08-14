#include "imbibe.h"

//#include "bitmap_graphics.h"
#include "bitmap_g.h"

#include <string.h>

#include "bitmap.h"
#include "color.h"
#include "graphics.h"
#include "pixel.h"


bitmap_graphics::bitmap_graphics(bitmap & n_b)
  : graphics(0, 0, n_b.width(), n_b.height()), m_b(n_b)
{
}


void bitmap_graphics::draw_rectangle(coord_t x1, coord_t y1, coord_t x2,
    coord_t y2, pixel p) {
  assert(clip_x1() >= 0); assert(clip_y1() >= 0);
  assert(clip_x2() >= 0); assert(clip_y2() >= 0);
  assert_margin(x1, COORD_MAX); assert_margin(y1, COORD_MAX);
  assert_margin(x2, COORD_MAX); assert_margin(y2, COORD_MAX);

  if ((x1 == x2) || (x1 >= clip_x2()) || (x2 <= clip_x1())
      || (y1 == y2) || (y1 >= clip_y2()) || (y2 <= clip_y1())) {
    return;
  }

  coord_t cx1 = min(x1, clip_x1());
  coord_t cy1 = min(y1, clip_y1());
  coord_t cx2 = max(x2, clip_x2());
  coord_t cy2 = max(y2, clip_y2());

  uint16_t rows = cy2 - cy1;
  uint16_t stride = m_b.width();
  uint16_t cols = cx2 - cx1;
  uint16_t * pp = m_b.data() + cy1 * stride + cx1;
  for (uint16_t r = 0; r < rows; ++r) {
    for (uint16_t c = 0; c < cols; ++c) {
      pp[c] = p;
    }
    pp += 80;
  }
}


void bitmap_graphics::draw_text(coord_t x, coord_t y, color c,
    char const * s) {
  assert(clip_x1() >= 0); assert(clip_y1() >= 0);
  assert(clip_x2() >= 0); assert(clip_y2() >= 0);
  coord_t i;
  coord_t s_len = strlen(s);

  x += origin_x();
  y += origin_y();

  if((y < clip_y2()) && (y >= clip_y1())) {
    if((x < clip_x2()) && (x + s_len > clip_x1())) {
      if(x < clip_x1()) {
        s += clip_x1() - x;
        s_len -= clip_x1() - x;
        x = clip_x1();
      }
      if(x + s_len > clip_x2()) {
        s_len = clip_x2() - x;
      }
      for(i = 0; i < s_len; ++i) {
        m_b.at(x + i, y) = pixel(*s, c);
        ++s;
      }
    }
  }
}


void bitmap_graphics::draw_bitmap(coord_t x, coord_t y, bitmap const & b) {
  assert(clip_x1() >= 0); assert(clip_y1() >= 0);
  assert(clip_x2() >= 0); assert(clip_y2() >= 0);
  assert_margin(x, COORD_MAX); assert_margin(y, COORD_MAX);
  assert(&b != &m_b);

  coord_t dest_x = origin_x() + x;
  coord_t dest_y = origin_y() + y;
  coord_t source_x1 = 0;
  coord_t source_y1 = 0;
  coord_t source_x2 = b.width();
  coord_t source_y2 = b.height();

  if(dest_x < clip_x1()) {
    source_x1 += clip_x1() - dest_x;
    dest_x = clip_x1();
  }
  if(dest_y < clip_y1()) {
    source_y1 += clip_y1() - dest_y;
    dest_y = clip_y1();
  }
  if(dest_x + b.width() > clip_x2()) {
    source_x2 -= dest_x + b.width() - clip_x2() - source_x1;
  }
  if(dest_y + b.height() > clip_y2()) {
    source_y2 -= dest_y + b.height() - clip_y2() - source_y1;
  }

  if ((source_x2 <= source_x1) || (source_y2 <= source_y1)) {
    return;
  }

  m_b.copy_bitmap(dest_x, dest_y, b, source_x1, source_y1, source_x2, source_y2);
}


