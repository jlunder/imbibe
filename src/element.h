#ifndef __ELEMENT_H_INCLUDED
#define __ELEMENT_H_INCLUDED

#include "imbibe.h"

class element;

#include "graphics.h"
#include "window.h"

class element {
public:
  element() : m_frame(0, 0, 0, 0), m_z(0), m_owner(NULL), m_visible(false) {}
  virtual ~element() {}
  rect const &frame() const { return m_frame; }
  coord_t frame_z() const { return m_z; }
  bool has_owner() const { return (bool)m_owner; }
  window *owner() {
    assert(m_owner != NULL);
    return m_owner;
  }
  bool visible() const { return m_visible; }
  void set_frame_pos(coord_t x1, coord_t y1);
  void set_frame_size(coord_t width, coord_t height);
  void set_frame_depth(coord_t z);
  void set_frame(coord_t x1, coord_t y1, coord_t x2, coord_t y2) {
    set_frame(rect(x1, y1, x2, y2), m_z);
  }
  void set_frame(coord_t x1, coord_t y1, coord_t x2, coord_t y2, coord_t z) {
    set_frame(rect(x1, y1, x2, y2), z);
  }
  void set_frame(rect const &n_frame) { set_frame(n_frame, m_z); }
  void set_frame(rect const &n_frame, coord_t z);
  void set_owner(window *n_owner);
  void set_visible(bool n_visible);
  void show() { set_visible(true); }
  void hide() { set_visible(false); }
  void request_repaint();
  void request_repaint(rect const &r);

  virtual void paint(graphics *g) = 0;

private:
  rect m_frame;
  coord_t m_z;
  window *m_owner;
  bool m_visible;

#ifndef NDEBUG
  virtual void owner_changing() {}
#endif
};

#endif // __ELEMENT_H_INCLUDED
