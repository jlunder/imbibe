#ifndef __WINDOW_ELEMENT_H_INCLUDED
#define __WINDOW_ELEMENT_H_INCLUDED


#include "element.h"
#include "map.h"
#include "vector.h"
#include "window.h"


class window_element: public element, public window {
public:
  virtual ~window_element();

  virtual void paint(graphics & g);

  virtual void lock_repaint();
  virtual void unlock_repaint();
  virtual void repaint(coord_t x1, coord_t y1, coord_t x2, coord_t y2);
  virtual void add_element(element & e);
  virtual void remove_element(element & e);
  virtual void element_frame_changed(element & e, coord_t old_x1,
    coord_t old_y1, coord_t old_x2, coord_t old_y2, coord_t old_z);

  coord_t offset_x() { return m_offset_x; }
  coord_t offset_y() { return m_offset_y; }

  void set_offset_pos(coord_t offset_x, coord_t offset_y);

private:
  typedef map<coord_t, element *> element_list;
  typedef element_list::value_type element_list_value;
  typedef element_list::iterator element_list_iterator;

  element_list m_elements;

  void paint_element(graphics & g, element & e);

  coord_t m_offset_x;
  coord_t m_offset_y;

#ifndef NDEBUG
  int8_t m_lock_count;

  virtual void owner_changing() { assert(m_lock_count == 0); }
#endif
};


#endif // __WINDOW_ELEMENT_H_INCLUDED

