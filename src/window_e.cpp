#include "imbibe.h"

#include "window_e.h"


#define logf_window_element cprintf


window_element::~window_element() {
}


void window_element::paint(graphics & g) {
  for (element_list_iterator i = m_elements.begin(); i != m_elements.end();
      ++i) {
    element & e = *i->ref;
    graphics::subregion_state s;

    g.enter_subregion(s, e.frame_x1(), e.frame_y1(), e.frame_x1(),
      e.frame_y1(), e.frame_x2(), e.frame_y2());
    if(!g.subregion_trivial()) {
      logf_window_element("window_element paint %p\n", &e);
      e.paint(g);
    } else {
      logf_window_element("window_element clipped out %p\n", &e);
    }
    g.leave_subregion(s);
  }
}


void window_element::lock_repaint() {
#ifndef NDEBUG
  ++m_lock_count;
#endif
  assert(has_owner());
  owner().lock_repaint();
}


void window_element::unlock_repaint() {
#ifndef NDEBUG
  --m_lock_count;
#endif
  assert(has_owner());
  owner().unlock_repaint();
}


void window_element::repaint(coord_t x1, coord_t y1, coord_t x2, coord_t y2) {
  owner().repaint(frame_x1() + x1, frame_y1() + y1, frame_x1() + x2,
    frame_y1() + y2);
}


void window_element::add_element(element & e) {
  m_elements.insert(element_list_value(e.frame_z(), &e));
  repaint(e.frame_x1(), e.frame_y1(), e.frame_x2(), e.frame_y2());
}


void window_element::remove_element(element & e) {
  coord_t z = e.frame_z();
  element_list_iterator i = m_elements.lower_bound(z);
  while((i->key == z) && (i->ref != &e)) {
    ++i;
    assert(i != m_elements.end());
  }
  m_elements.erase(i);
#ifndef NDEBUG
  for (element_list_iterator j = m_elements.begin(); j != m_elements.end(); ++j) {
    assert(j->ref != &e);
  }
#endif

  repaint(e.frame_x1(), e.frame_y1(), e.frame_x2(), e.frame_y2());
}


void window_element::element_frame_changed(element & e, coord_t old_x1,
    coord_t old_y1, coord_t old_x2, coord_t old_y2, coord_t old_z)
{
  if(e.frame_z() != old_z)
  {
    element_list_iterator i = m_elements.lower_bound(old_z);
    while((i->key == old_z) && (i->ref != &e)) {
      ++i;
      assert(i != m_elements.end());
    }
    m_elements.erase(i);
#ifndef NDEBUG
    for (element_list_iterator j = m_elements.begin(); j != m_elements.end(); ++j) {
      assert(j->ref != &e);
    }
#endif
    m_elements.insert(element_list_value(e.frame_z(), &e));
  }
  owner().lock_repaint();
  request_repaint(old_x1, old_y1, old_x2, old_y2);
  request_repaint(e.frame_x1(), e.frame_y1(), e.frame_x2(), e.frame_y2());
  owner().unlock_repaint();
}


