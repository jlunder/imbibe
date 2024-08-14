#include "imbibe.h"

#include "graphics.h"


#define logf_graphics(...) disable_logf("GRAPHICS: " __VA_ARGS__)


graphics::graphics(coord_t n_clip_x1, coord_t n_clip_y1, coord_t n_clip_x2,
    coord_t n_clip_y2)
  : m_x(0), m_y(0), m_clip_x1(n_clip_x1), m_clip_y1(n_clip_y1),
    m_clip_x2(n_clip_x2), m_clip_y2(n_clip_y2) {
  assert_margin(n_clip_x1, COORD_MAX); assert_margin(n_clip_y1, COORD_MAX);
  assert_margin(n_clip_x2, COORD_MAX); assert_margin(n_clip_y2, COORD_MAX);
  assert(n_clip_x1 >= 0); assert(n_clip_y1 >= 0);
  assert(n_clip_x1 <= n_clip_x2); assert(n_clip_y1 <= n_clip_y2);

#ifndef NDEBUG
  m_subregion_depth = 0;
#endif
}


graphics::graphics()
  : m_x(0), m_y(0), m_clip_x1(0), m_clip_y1(0),
    m_clip_x2(COORD_MAX / 4), m_clip_y2(COORD_MAX / 4) {
#ifndef NDEBUG
  m_subregion_depth = 0;
#endif
}


void graphics::enter_subregion(subregion_state & save, coord_t x, coord_t y,
    coord_t clip_x1, coord_t clip_y1, coord_t clip_x2, coord_t clip_y2) {
  assert_margin(x, COORD_MAX); assert_margin(y, COORD_MAX);
  assert_margin(clip_x1, COORD_MAX); assert_margin(clip_y1, COORD_MAX);
  assert_margin(clip_x2, COORD_MAX); assert_margin(clip_y2, COORD_MAX);
  assert(clip_x1 <= clip_x2); assert(clip_y1 <= clip_y2);

  logf_graphics("graphics %p enter_subregion %d, %d, %d, %d, %d, %d\n", this,
    x, y, clip_x1, clip_y1, clip_x2, clip_y2);

  save.m_x = m_x;
  save.m_y = m_y;
  save.m_clip_x1 = m_clip_x1;
  save.m_clip_y1 = m_clip_y1;
  save.m_clip_x2 = m_clip_x2;
  save.m_clip_y2 = m_clip_y2;
#ifndef NDEBUG
  save.m_subregion_depth = m_subregion_depth;
  ++m_subregion_depth;
#endif

  m_clip_x1 = max<coord_t>(m_clip_x1, m_x + clip_x1);
  m_clip_y1 = max<coord_t>(m_clip_y1, m_y + clip_y1);
  m_clip_x2 = min<coord_t>(m_clip_x2, m_x + clip_x2);
  m_clip_y2 = min<coord_t>(m_clip_y2, m_y + clip_y2);
  m_x += x;
  m_y += y;

  logf_graphics("  clip %d, %d, %d, %d\n", m_clip_x1, m_clip_y1, m_clip_x2,
    m_clip_y2);
}


void graphics::leave_subregion(subregion_state const & restore) {
#ifndef NDEBUG
  --m_subregion_depth;
  assert(restore.m_subregion_depth == m_subregion_depth);
#endif
  m_x = restore.m_x;
  m_y = restore.m_y;
  m_clip_x1 = restore.m_clip_x1;
  m_clip_y1 = restore.m_clip_y1;
  m_clip_x2 = restore.m_clip_x2;
  m_clip_y2 = restore.m_clip_y2;

  logf_graphics("graphics %p leave_subregion\n", this);
  logf_graphics("  clip %d, %d, %d, %d\n", m_clip_x1, m_clip_y1,
    m_clip_x2, m_clip_y2);
}


