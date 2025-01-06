#ifndef __WINDOW_H_INCLUDED
#define __WINDOW_H_INCLUDED

#include "imbibe.h"

class window;

#include "element.h"
#include "graphics.h"

class window {
public:
  virtual ~window() {}

  virtual void lock_repaint() = 0;
  virtual void unlock_repaint() = 0;
  virtual void repaint(rect const &r) = 0;
  virtual void add_element(element *e) = 0;
  virtual void remove_element(element *e) = 0;
  virtual void element_frame_changed(element *e, rect const &old_frame,
                                     coord_t old_z) = 0;
};

#endif // __WINDOW_H_INCLUDED
