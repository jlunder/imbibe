#ifndef __TEXT_WINDOW_HH_INCLUDED
#define __TEXT_WINDOW_HH_INCLUDED


#include "imbibe.h"


class text_window;


#include "element.h"
// #include "functional.h"
#include "function.h"
#include "graphics.h"
#include "map.h"
#include "window.h"


class text_window: public window
{
public:
  text_window();
  virtual ~text_window();
  virtual void lock();
  virtual void unlock();
  virtual void repaint();
  virtual void repaint(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
  virtual void repaint(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
    int16_t z);
  virtual void add_element(element & e);
  virtual void remove_element(element & e);
  virtual void element_frame_pos_changed(element & e, int16_t old_x1,
    int16_t old_y1);
  virtual void element_frame_size_changed(element & e, int16_t old_width,
    int16_t old_height);
  virtual void element_frame_depth_changed(element & e, int16_t old_z);
  virtual void element_frame_changed(element & e, int16_t old_x1,
    int16_t old_y1, int16_t old_x2, int16_t old_y2, int16_t old_z);

private:
  typedef map<int16_t, element *, less<int16_t> > element_list;
  typedef map<int16_t, element *, less<int16_t> >::value_type
    element_list_value;
  typedef map<int16_t, element *, less<int16_t> >::iterator
    element_list_iterator;

  void repaint_element(element const & e, int16_t x1, int16_t y1, int16_t x2,
    int16_t y2);
  void locked_repaint(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
  void locked_repaint(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
    int16_t z);

  element_list m_elements;
  bitmap * m_backbuffer;
  int8_t m_locked;
  bool m_need_repaint;
  int16_t m_repaint_x1;
  int16_t m_repaint_y1;
  int16_t m_repaint_x2;
  int16_t m_repaint_y2;
  int16_t m_repaint_z;
  bool m_repaint_z_minus_infinity;

  static void save_mode();
  static void restore_mode();
  static void set_text_mode();
  static void flip(bitmap * backbuffer);
};


#endif //__TEXT_WINDOW_HH_INCLUDED


