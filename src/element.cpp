#include "imbibe.h"

#include "element.h"

#include "window.h"


#define logf_element(...) disable_logf("ELEMENT: " __VA_ARGS__)


void element::set_frame_pos(coord_t x1, coord_t y1) {
  assert_margin(x1, COORD_MAX); assert_margin(y1, COORD_MAX);

  if((x1 == m_x1) && (y1 == m_y1)) {
    return;
  }

  coord_t old_x1 = m_x1;
  coord_t old_y1 = m_y1;
  coord_t old_x2 = m_x2;
  coord_t old_y2 = m_y2;
  m_x1 = x1;
  m_y1 = y1;
  m_x2 = x1 + (old_x2 - old_x1);
  m_y2 = y1 + (old_y2 - old_y1);
  if (m_visible && m_owner) {
    m_owner->element_frame_changed(*this, old_x1, old_y1, old_x2, old_y2,
      m_z);
  }
}


void element::set_frame_size(coord_t width, coord_t height) {
  assert_margin(width, COORD_MAX); assert_margin(height, COORD_MAX);
  assert(width >= 0); assert(height >= 0);

  if((width == frame_width()) && (height == frame_height())) {
    return;
  }

  coord_t old_x2 = m_x2;
  coord_t old_y2 = m_y2;
  m_x2 = m_x1 + width;
  m_y2 = m_y1 + height;
  assert_margin(m_x2, COORD_MAX); assert_margin(m_y2, COORD_MAX);
  if(m_visible && m_owner) {
    m_owner->element_frame_changed(*this, m_x1, m_y2, old_x2, old_y2, m_z);
  }
}


void element::set_frame_depth(coord_t z) {
  assert_margin(z, COORD_MAX);

  if (z == m_z) {
    return;
  }

  coord_t old_z = m_z;
  m_z = z;
  if (m_visible && m_owner) {
    m_owner->element_frame_changed(*this, m_x1, m_y1, m_x2, m_y2, old_z);
  }
}


void element::set_frame(coord_t x1, coord_t y1, coord_t x2, coord_t y2,
    coord_t z) {
  assert_margin(x1, COORD_MAX); assert_margin(y1, COORD_MAX);
  assert_margin(x2, COORD_MAX); assert_margin(y2, COORD_MAX);
  assert_margin(z, COORD_MAX);
  assert(x1 <= x2); assert(y1 <= y2);

  if ((x1 == m_x1) && (y1 == m_y1) && (x2 == m_x2) && (y2 == m_y2)) {
    return;
  }

  coord_t old_x1 = m_x1;
  coord_t old_y1 = m_y1;
  coord_t old_x2 = m_x2;
  coord_t old_y2 = m_y2;
  coord_t old_z = m_z;
  m_x1 = x1;
  m_y1 = y1;
  m_x2 = x2;
  m_y2 = y2;
  m_z = z;
  if (m_visible && m_owner) {
    m_owner->element_frame_changed(*this, old_x1, old_y1, old_x2, old_y2,
      old_z);
  }
}


void element::set_owner(window & n_owner) {
#ifndef NDEBUG
  owner_changing();
#endif

  if (m_visible && m_owner) {
    m_owner->remove_element(*this);
  }
  m_owner = &n_owner;
  if (m_visible && m_owner) {
    m_owner->add_element(*this);
  }
}


void element::show() {
  bool was_visible = m_visible;
  m_visible = true;
  if (!was_visible && m_owner) {
    m_owner->add_element(*this);
  }
}


void element::hide() {
  if (m_visible && m_owner) {
    m_owner->remove_element(*this);
  }
  m_visible = false;
}


void element::request_repaint() {
  if (m_visible && m_owner) {
    logf_element("element %p request_repaint %d, %d, %d, %d\n",
      this, 0, 0, frame_width(), frame_height());
    m_owner->repaint(m_x1, m_y1, m_x2, m_y2);
  }
}


void element::request_repaint(coord_t x1, coord_t y1, coord_t x2,
    coord_t y2) {
  assert_margin(x1, COORD_MAX); assert_margin(y1, COORD_MAX);
  assert_margin(x2, COORD_MAX); assert_margin(y2, COORD_MAX);
  if (m_visible && m_owner && (x1 < x2) && (y1 < y2)) {
    logf_element("element %p request_repaint %d, %d, %d, %d\n",
      this, x1, y1, x2, y2);
    m_owner->repaint(m_x1 + max<coord_t>(x1, 0), m_y1 + max<coord_t>(y1, 0),
      min<coord_t>(m_x1 + x2, m_x2), min<coord_t>(m_y1 + y2, m_y2));
  }
}


void element::animate(uint32_t delta_ms) {
  (void)delta_ms;
}


bool element::handle_key(uint16_t key) {
  (void)key;
  return false;
}


