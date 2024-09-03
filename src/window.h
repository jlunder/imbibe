#ifndef __WINDOW_H_INCLUDED
#define __WINDOW_H_INCLUDED

#include "imbibe.h"

class window;

#include "element.h"
#include "graphics.h"

class window {
public:
  virtual void lock_repaint() = 0;
  virtual void unlock_repaint() = 0;
  virtual void repaint(coord_t x1, coord_t y1, coord_t x2, coord_t y2) = 0;
  virtual void add_element(element *e) = 0;
  virtual void remove_element(element *e) = 0;
  virtual void element_frame_changed(element *e, coord_t old_x1, coord_t old_y1,
                                     coord_t old_x2, coord_t old_y2,
                                     coord_t old_z) = 0;
};

#endif // __WINDOW_H_INCLUDED
