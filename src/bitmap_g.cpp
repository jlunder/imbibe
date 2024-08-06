#include "imbibe.h"

//#include "bitmap_graphics.h"
#include "bitmap_g.h"

#include <string.h>

#include "bitmap.h"
#include "color.h"
#include "graphics.h"
#include "pixel.h"


bitmap_graphics::bitmap_graphics(bitmap & n_b)
  : m_b(n_b), m_clip_x1(0), m_clip_y1(0), m_clip_x2(n_b.width()),
    m_clip_y2(n_b.height()), m_bounds_x1(0), m_bounds_y1(0),
    m_bounds_x2(n_b.width()), m_bounds_y2(n_b.height())
{
}


int16_t bitmap_graphics::width() const {
  return m_b.width();
}


int16_t bitmap_graphics::height() const {
  return m_b.height();
}


int16_t bitmap_graphics::bounds_x1() const {
  return m_bounds_x1;
}


int16_t bitmap_graphics::bounds_y1() const {
  return m_bounds_y1;
}


int16_t bitmap_graphics::bounds_x2() const {
  return m_bounds_x2;
}


int16_t bitmap_graphics::bounds_y2() const {
  return m_bounds_y2;
}


int16_t bitmap_graphics::bounds_width() const {
  return m_bounds_x2 - m_bounds_x1;
}


int16_t bitmap_graphics::bounds_height() const {
  return m_bounds_y2 - m_bounds_y1;
}


int16_t bitmap_graphics::clip_x1() const {
  return m_clip_x1;
}


int16_t bitmap_graphics::clip_y1() const {
  return m_clip_y1;
}


int16_t bitmap_graphics::clip_x2() const {
  return m_clip_x2;
}


int16_t bitmap_graphics::clip_y2() const {
  return m_clip_y2;
}


void bitmap_graphics::set_bounds(int16_t x1, int16_t y1, int16_t x2,
    int16_t y2) {
  assert(x1 <= x2); assert(y1 <= y2);
  m_bounds_x1 = x1;
  m_bounds_y1 = y1;
  m_bounds_x2 = x2;
  m_bounds_y2 = y2;
}


void bitmap_graphics::set_clip(int16_t x1, int16_t y1, int16_t x2,
    int16_t y2) {
  assert(x1 <= x2); assert(y1 <= y2);
  m_clip_x1 = max<int16_t>(max(m_bounds_x1, x1), 0);
  m_clip_y1 = max<int16_t>(max(m_bounds_y1, y1), 0);
  m_clip_x2 = min(min(x2, m_bounds_x2), m_b.width());
  m_clip_y2 = min(min(y2, m_bounds_y2), m_b.height());
}


void bitmap_graphics::draw_rectangle(int16_t x1, int16_t y1, int16_t x2,
    int16_t y2, pixel p) {
  if ((x1 == x2) || (x1 >= m_clip_x2) || (x2 <= m_clip_x1)
      || (y1 == y2) || (y1 >= m_clip_y2) || (y2 <= m_clip_y1)) {
    return;
  }

  int16_t cx1 = min(x1, m_clip_x1);
  int16_t cy1 = min(y1, m_clip_y1);
  int16_t cx2 = max(x2, m_clip_x2);
  int16_t cy2 = max(y2, m_clip_y2);

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


void bitmap_graphics::draw_text(int16_t x, int16_t y, color c,
    char const * s) {
  int16_t i;
  int16_t s_len = strlen(s);

  x += m_bounds_x1;
  y += m_bounds_y1;

  if((y < m_clip_y2) && (y >= m_clip_y1)) {
    if((x < m_clip_x2) && (x + s_len > m_clip_x1)) {
      if(x < m_clip_x1) {
        s += m_clip_x1 - x;
        s_len -= m_clip_x1 - x;
        x = m_clip_x1;
      }
      if(x + s_len > m_clip_x2) {
        s_len = m_clip_x2 - x;
      }
      for(i = 0; i < s_len; ++i) {
        m_b.at(x + i, y) = pixel(*s, c);
        ++s;
      }
    }
  }
}


void bitmap_graphics::draw_bitmap(int16_t x, int16_t y, bitmap const & b) {
  int16_t dest_x = x + m_bounds_x1;
  int16_t dest_y = y + m_bounds_y1;
  int16_t source_x1 = 0;
  int16_t source_y1 = 0;
  int16_t source_x2 = b.width();
  int16_t source_y2 = b.height();

  if((dest_y < m_clip_y2) && (dest_y + b.height() > m_clip_y1)) {
    if((dest_x < m_clip_x2) && (dest_x + b.width() > m_clip_x1)) {
      if(dest_x < m_clip_x1) {
        source_x1 += m_clip_x1 - dest_x;
        dest_x = m_clip_x1;
      }
      if(dest_y < m_clip_y1) {
        source_y1 += m_clip_y1 - dest_y;
        dest_y = m_clip_y1;
      }
      if(dest_x + b.width() > m_clip_x2) {
        source_x2 -= dest_x + b.width() - m_clip_x2 - source_x1;
      }
      if(dest_y + b.height() > m_clip_y2) {
        source_y2 -= dest_y + b.height() - m_clip_y2 - source_y1;
      }
      if(&b != &m_b) {
        m_b.copy_bitmap(dest_x, dest_y, b, source_x1, source_y1, source_x2, source_y2);
      } else {
        m_b.copy_this_bitmap(dest_x, dest_y, source_x1, source_y1, source_x2, source_y2);
      }
    }
  }
}


