#include "imbibe.h"

#include "graphics.h"


void graphics::set_bounds(int16_t x1, int16_t y1, int16_t x2,
    int16_t y2) {
  assert_margin(x1, INT16_MAX); assert_margin(y1, INT16_MAX);
  assert_margin(x2, INT16_MAX); assert_margin(y2, INT16_MAX);
  assert(x1 <= x2); assert(y1 <= y2);
  m_bounds_x1 = x1;
  m_bounds_y1 = y1;
  m_bounds_x2 = x2;
  m_bounds_y2 = y2;
#ifndef NDEBUG
  m_clip_x1 = -1;
  m_clip_y1 = -1;
  m_clip_x2 = -1;
  m_clip_y2 = -1;
#endif
}


void graphics::set_clip(int16_t x1, int16_t y1, int16_t x2,
    int16_t y2) {
  assert_margin(x1, INT16_MAX); assert_margin(y1, INT16_MAX);
  assert_margin(x2, INT16_MAX); assert_margin(y2, INT16_MAX);
  assert(x1 <= x2); assert(y1 <= y2);
  m_clip_x1 = max<int16_t>(x1, m_bounds_x1);
  m_clip_y1 = max<int16_t>(y1, m_bounds_y1);
  m_clip_x2 = min<int16_t>(x2, m_bounds_x2);
  m_clip_y2 = min<int16_t>(y2, m_bounds_y2);
}


