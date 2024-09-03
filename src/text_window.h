#ifndef __TEXT_WINDOW_H_INCLUDED
#define __TEXT_WINDOW_H_INCLUDED

#include "imbibe.h"

#include "bitmap.h"
#include "map.h"
#include "window.h"

class element;
class graphics;

class text_window : public window {
public:
  text_window();

  void setup(bitmap *capture_screen = NULL);
  void teardown();

  void present();

  bitmap *backbuffer() { return &m_backbuffer; }

  virtual void lock_repaint();
  virtual void unlock_repaint();
  virtual void repaint(rect const &r);
  virtual void add_element(element *e);
  virtual void remove_element(element *e);
  virtual void element_frame_changed(element *e, rect const &old_frame,
                                     coord_t old_z);

private:
  void paint_element(graphics *g, element *e);
  void locked_repaint(rect const &r);
  void locked_repaint(rect const &r, coord_t z);

  element *m_element;
  bitmap m_backbuffer;
  int8_t m_lock_count;
  bool m_need_repaint;
  rect m_repaint;
  bool m_dirty;
  rect m_dirty_bb;

  static void present_copy(termel_t const *backbuffer, coord_t width,
                           coord_t height, rect const &r);
};

extern text_window text_window_instance;

#endif // __TEXT_WINDOW_H_INCLUDED
