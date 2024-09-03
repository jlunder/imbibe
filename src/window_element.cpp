#include "imbibe.h"

#include "window_element.h"

#define logf_window_element(...) disable_logf("WINDOW_ELEMENT: " __VA_ARGS__)

window_element::window_element() {
  m_elements.mem(arena::cur());
}

void window_element::paint(graphics *g) {
#ifndef NDEBUG
  assert(m_lock_count == 0);
#endif
  for (element_list_iterator i = m_elements.begin(); i != m_elements.end();
       ++i) {
    element *e = i->ref;
    graphics::subregion_state s;

    g->enter_subregion(m_offset_x + e->frame_x1(), m_offset_y + e->frame_y1(),
                       m_offset_x + e->frame_x1(), m_offset_y + e->frame_y1(),
                       m_offset_x + e->frame_x2(), m_offset_y + e->frame_y2(),
                       &s);
    if (!g->subregion_trivial()) {
      logf_window_element("window_element paint %p\n", e);
      e->paint(g);
    } else {
      logf_window_element("window_element clipped out %p\n", e);
    }
    g->leave_subregion(&s);
  }
}

void window_element::lock_repaint() {
#ifndef NDEBUG
  ++m_lock_count;
#endif
  assert(has_owner());
  owner()->lock_repaint();
}

void window_element::unlock_repaint() {
#ifndef NDEBUG
  --m_lock_count;
  assert(m_lock_count >= 0);
#endif
  assert(has_owner());
  owner()->unlock_repaint();
}

void window_element::repaint(coord_t x1, coord_t y1, coord_t x2, coord_t y2) {
  request_repaint(x1, y1, x2, y2);
}

void window_element::add_element(element *e) {
  assert(e->owner() == this);
  assert(e->visible());
  m_elements.insert(element_list_value(e->frame_z(), e));
  request_repaint(m_offset_x + e->frame_x1(), m_offset_y + e->frame_y1(),
                  m_offset_x + e->frame_x2(), m_offset_y + e->frame_y2());
}

void window_element::remove_element(element *e) {
  assert(e->owner() == this);
  assert(e->visible());
  coord_t z = e->frame_z();
  element_list_iterator i = m_elements.lower_bound(z);
  while ((i->key == z) && (i->ref != e)) {
    ++i;
    assert(i != m_elements.end());
  }
  m_elements.erase(i);
#ifndef NDEBUG
  for (element_list_iterator j = m_elements.begin(); j != m_elements.end();
       ++j) {
    assert(j->ref != e);
  }
#endif

  request_repaint(m_offset_x + e->frame_x1(), m_offset_y + e->frame_y1(),
                  m_offset_x + e->frame_x2(), m_offset_y + e->frame_y2());
}

void window_element::element_frame_changed(element *e, coord_t old_x1,
                                           coord_t old_y1, coord_t old_x2,
                                           coord_t old_y2, coord_t old_z) {
  if (e->frame_z() != old_z) {
    element_list_iterator i = m_elements.lower_bound(old_z);
    while ((i->key == old_z) && (i->ref != e)) {
      ++i;
      assert(i != m_elements.end());
    }
    m_elements.erase(i);
#ifndef NDEBUG
    for (element_list_iterator j = m_elements.begin(); j != m_elements.end();
         ++j) {
      assert(j->ref != e);
    }
#endif
    m_elements.insert(element_list_value(e->frame_z(), e));
  }
  owner()->lock_repaint();
  request_repaint(m_offset_x + old_x1, m_offset_y + old_y1, m_offset_x + old_x2,
                  m_offset_y + old_y2);
  request_repaint(m_offset_x + e->frame_x1(), m_offset_y + e->frame_y1(),
                  m_offset_x + e->frame_x2(), m_offset_y + e->frame_y2());
  owner()->unlock_repaint();
}

void window_element::set_offset_pos(coord_t offset_x, coord_t offset_y) {
  assert_margin(offset_x, COORD_MAX);
  assert_margin(offset_y, COORD_MAX);

  if ((offset_x == m_offset_x) && (offset_y == m_offset_y)) {
    return;
  }

  m_offset_x = offset_x;
  m_offset_y = offset_y;
  if (!m_elements.empty()) {
    request_repaint();
  }
}
