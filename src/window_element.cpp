#include "imbibe.h"

#include "window_element.h"

#define logf_window_element(...) disable_logf("WINDOW_ELEMENT: " __VA_ARGS__)

window_element::window_element() {}

void window_element::paint(graphics *g) {
#ifndef NDEBUG
  assert(m_lock_count == 0);
#endif
  for (element_list_iterator i = m_elements.begin(); i != m_elements.end();
       ++i) {
    element *e = i->ref;
    graphics::subregion_state s;

    g->enter_subregion(m_offset + point(e->frame().x1, e->frame().y1),
                       e->frame(), &s);
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

void window_element::repaint(rect const &r) { request_repaint(r); }

void window_element::add_element(element *e) {
  assert(e->owner() == this);
  assert(e->visible());
  m_elements.insert(element_list_value(e->frame_z(), e));
  request_repaint(m_offset + e->frame());
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

  request_repaint(m_offset + e->frame());
}

void window_element::element_frame_changed(element *e, rect const &old_frame,
                                           coord_t old_z) {
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
  if (e->frame().overlaps(old_frame)) {
    request_repaint(m_offset + (e->frame() | old_frame));
  } else {
    request_repaint(m_offset + old_frame);
    request_repaint(m_offset + e->frame());
  }
}

void window_element::set_offset(point n_offset) {
  assert_margin(n_offset.x, COORD_MAX);
  assert_margin(n_offset.y, COORD_MAX);

  if (m_offset == n_offset) {
    return;
  }

  m_offset = n_offset;
  if (!m_elements.empty()) {
    request_repaint();
  }
}
