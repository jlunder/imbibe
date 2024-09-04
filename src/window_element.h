#ifndef __WINDOW_ELEMENT_H_INCLUDED
#define __WINDOW_ELEMENT_H_INCLUDED

#include "element.h"
#include "map.h"
#include "vector.h"
#include "window.h"

class window_element : public element, public window {
public:
  window_element();
  virtual ~window_element() {
#if BUILD_DEBUG
    assert(m_lock_count == 0);
#endif
  }

  virtual void paint(graphics *g);

  virtual void lock_repaint();
  virtual void unlock_repaint();
  virtual void repaint(rect const &r);
  virtual void add_element(element *e);
  virtual void remove_element(element *e);
  virtual void element_frame_changed(element *e, rect const &old_frame,
                                     coord_t old_z);

  point offset() { return m_offset; }

  void set_offset(point n_offset);

private:
  typedef map<coord_t, element *> element_list;
  typedef element_list::value_type element_list_value;
  typedef element_list::iterator element_list_iterator;

  element_list m_elements;

  void paint_element(graphics *g, element *e);

  point m_offset;

#if BUILD_DEBUG
  int8_t m_lock_count;

  virtual void owner_changing() { assert(m_lock_count == 0); }
#endif
};

#endif // __WINDOW_ELEMENT_H_INCLUDED
