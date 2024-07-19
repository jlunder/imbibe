#ifndef __WINDOW_HH_INCLUDED
#define __WINDOW_HH_INCLUDED


class window;


#include <stddef.h>

#include "element.hh"
#include "graphics.hh"


class window
{
public:
  virtual void lock() = 0;
  virtual void unlock() = 0;
  virtual void repaint() = 0;
  virtual void repaint(int x1, int y1, int x2, int y2) = 0;
  virtual void repaint(int x1, int y1, int x2, int y2, int z) = 0;
  virtual void add_element(element & e) = 0;
  virtual void remove_element(element & e) = 0;
  virtual void element_frame_pos_changed(element & e, int old_x1, int old_y1) = 0;
  virtual void element_frame_size_changed(element & e, int old_width, int old_height) = 0;
  virtual void element_frame_depth_changed(element & e, int old_z) = 0;
  virtual void element_frame_changed(element & e, int old_x1, int old_y1, int old_x2, int old_y2, int old_z) = 0;
};


//#include "window.ii"


#endif //__WINDOW_HH_INCLUDED


