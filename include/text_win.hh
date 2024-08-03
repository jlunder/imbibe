#ifndef __TEXT_WINDOW_HH_INCLUDED
#define __TEXT_WINDOW_HH_INCLUDED


#include "imbibe.hh"


class text_window;


#include "element.hh"
#include "functional.hh"
#include "graphics.hh"
#include "map.hh"
#include "window.hh"


class text_window: public window
{
public:
  text_window();
  virtual ~text_window();
  virtual void lock();
  virtual void unlock();
  virtual void repaint();
  virtual void repaint(int x1, int y1, int x2, int y2);
  virtual void repaint(int x1, int y1, int x2, int y2, int z);
  virtual void add_element(element & e);
  virtual void remove_element(element & e);
  virtual void element_frame_pos_changed(element & e, int old_x1, int old_y1);
  virtual void element_frame_size_changed(element & e, int old_width, int old_height);
  virtual void element_frame_depth_changed(element & e, int old_z);
  virtual void element_frame_changed(element & e, int old_x1, int old_y1, int old_x2, int old_y2, int old_z);

private:
  typedef map < int, element *, less < int > > element_list;
  typedef map < int, element *, less < int > > ::value_type element_list_value;
  typedef map < int, element *, less < int > > ::iterator element_list_iterator;

  void repaint_element(element const & e, int x1, int y1, int x2, int y2);
  void locked_repaint(int x1, int y1, int x2, int y2);
  void locked_repaint(int x1, int y1, int x2, int y2, int z);

  element_list m_elements;
  bitmap * m_backbuffer;
  bool m_locked;
  bool m_need_repaint;
  int m_repaint_x1;
  int m_repaint_y1;
  int m_repaint_x2;
  int m_repaint_y2;
  int m_repaint_z;
  bool m_repaint_z_minus_infinity;

  static void save_mode();
  static void restore_mode();
  static void set_text_mode();
  static void flip(bitmap * backbuffer);
};


//#include "text_window.ii"


#endif //__TEXT_WINDOW_HH_INCLUDED


