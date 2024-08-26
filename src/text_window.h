#ifndef __TEXT_WINDOW_H_INCLUDED
#define __TEXT_WINDOW_H_INCLUDED


#include "imbibe.h"


#include "bitmap.h"
#include "map.h"
#include "window.h"


class element;
class graphics;


class text_window: public window
{
public:
  text_window();

  void setup(bitmap * capture_screen = NULL);
  void teardown();

  void present();

  bitmap & backbuffer() { return m_backbuffer; }

  virtual void lock_repaint();
  virtual void unlock_repaint();
  virtual void repaint(coord_t x1, coord_t y1, coord_t x2, coord_t y2);
  virtual void add_element(element & e);
  virtual void remove_element(element & e);
  virtual void element_frame_changed(element & e, coord_t old_x1,
    coord_t old_y1, coord_t old_x2, coord_t old_y2, coord_t old_z);

  virtual bool is_element();
  virtual element & as_element();

  virtual void set_focus(element & e);
  virtual void clear_focus();
  virtual bool has_focus();
  virtual element & focus();

private:
  void paint_element(graphics & g, element & e);
  void locked_repaint(coord_t x1, coord_t y1, coord_t x2, coord_t y2);
  void locked_repaint(coord_t x1, coord_t y1, coord_t x2, coord_t y2,
    coord_t z);

  element * m_element;
  element * m_focus;
  bitmap m_backbuffer;
  int8_t m_lock_count;
  bool m_need_repaint;
  coord_t m_repaint_x1;
  coord_t m_repaint_y1;
  coord_t m_repaint_x2;
  coord_t m_repaint_y2;
  bool m_dirty;
  coord_t m_dirty_bb_x1;
  coord_t m_dirty_bb_y1;
  coord_t m_dirty_bb_x2;
  coord_t m_dirty_bb_y2;

  static void present_copy(termel_t const * backbuffer, coord_t width,
    coord_t height, coord_t x1, coord_t y1, coord_t x2, coord_t y2);
};


extern text_window text_window_instance;


#endif // __TEXT_WINDOW_H_INCLUDED


