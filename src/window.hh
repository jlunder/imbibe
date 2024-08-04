#ifndef __WINDOW_HH_INCLUDED
#define __WINDOW_HH_INCLUDED


#include "imbibe.hh"

class window;

#include "element.hh"
#include "graphics.hh"


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


#endif //__WINDOW_HH_INCLUDED


