#ifndef __WINDOW_H_INCLUDED
#define __WINDOW_H_INCLUDED


#include "imbibe.h"

class window;

#include "element.h"
#include "graphics.h"


class window
{
public:
  virtual void lock() = 0;
  virtual void unlock() = 0;
  virtual void repaint() = 0;
  virtual void repaint(int16_t x1, int16_t y1, int16_t x2, int16_t y2) = 0;
  virtual void repaint(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
    int16_t z) = 0;
  virtual void add_element(element & e) = 0;
  virtual void remove_element(element & e) = 0;
  virtual void element_frame_pos_changed(element & e, int16_t old_x1,
    int16_t old_y1) = 0;
  virtual void element_frame_size_changed(element & e, int16_t old_width,
    int16_t old_height) = 0;
  virtual void element_frame_depth_changed(element & e, int16_t old_z) = 0;
  virtual void element_frame_changed(element & e, int16_t old_x1,
    int16_t old_y1, int16_t old_x2, int16_t old_y2, int16_t old_z) = 0;
};


#endif // __WINDOW_H_INCLUDED


