#include "imbibe.h"

#include "element.h"

#include "window.h"

#define logf_element(...) disable_logf("ELEMENT: " __VA_ARGS__)

void element::set_frame_pos(coord_t x, coord_t y) {
  assert_margin(x, COORD_MAX);
  assert_margin(y, COORD_MAX);
  assert(m_frame.reasonable());

  if ((x == m_frame.x1) && (y == m_frame.y1)) {
    return;
  }

  rect old_frame = m_frame;
  m_frame.assign(x, y, x + m_frame.width(), y + m_frame.height());
  assert(m_frame.reasonable());
  if (m_visible && m_owner) {
    m_owner->element_frame_changed(this, old_frame, m_z);
  }
}

void element::set_frame_size(coord_t width, coord_t height) {
  assert_margin(width, COORD_MAX);
  assert_margin(height, COORD_MAX);
  assert(width >= 0);
  assert(height >= 0);
  assert(m_frame.reasonable());

  if ((width == m_frame.width()) && (height == m_frame.height())) {
    return;
  }

  rect old_frame = m_frame;
  m_frame.x2 = m_frame.x1 + width;
  m_frame.y2 = m_frame.y1 + height;
  assert(m_frame.reasonable());
  if (m_visible && m_owner) {
    m_owner->element_frame_changed(this, old_frame, m_z);
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
    m_owner->element_frame_changed(this, m_frame, old_z);
  }
}

void element::set_frame(rect const &n_frame, coord_t z) {
  assert(n_frame.reasonable());
  assert_margin(z, COORD_MAX);

  rect old_frame = m_frame;
  coord_t old_z = m_z;
  m_frame = n_frame;
  if (m_frame == old_frame) {
    return;
  }
  m_z = z;
  if (m_visible && m_owner) {
    m_owner->element_frame_changed(this, old_frame, old_z);
  }
}

void element::set_owner(window *n_owner) {
#if BUILD_DEBUG
  owner_changing();
#endif

  if (m_visible && m_owner) {
    m_owner->remove_element(this);
  }
  m_owner = n_owner;
  if (m_visible && m_owner) {
    m_owner->add_element(this);
  }
}

void element::set_visible(bool n_visible) {
  if (m_visible == n_visible) {
    return;
  }

  if (n_visible) {
    m_visible = true;
    if (m_owner) {
      m_owner->add_element(this);
    }
  } else {
    if (m_owner) {
      m_owner->remove_element(this);
    }
    m_visible = false;
  }
}

void element::request_repaint() {
  if (m_visible && m_owner) {
    logf_element("element %p request_repaint %d, %d, %d, %d\n", this, 0, 0,
                 frame_width(), frame_height());
    m_owner->repaint(m_frame);
  }
}

void element::request_repaint(rect const &r) {
  assert(r.reasonable());
  if (m_visible && m_owner && !r.trivial()) {
    logf_element("element %p request_repaint %d, %d, %d, %d\n", this, x1, y1,
                 x2, y2);
    m_owner->repaint(m_frame & (r + point(m_frame.x1, m_frame.y1)));
  }
}
