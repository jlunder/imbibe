#ifndef __ELEMENT_H_INCLUDED
#define __ELEMENT_H_INCLUDED


#include "imbibe.h"


class element;


#include "graphics.h"
#include "window.h"


class element
{
public:
  element(): m_x1(0), m_y1(0), m_x2(0), m_y2(0), m_z(0), m_owner(NULL), m_visible(false) { }
  virtual ~element() { }
  int16_t frame_x1() const { return m_x1; }
  int16_t frame_y1() const { return m_y1; }
  int16_t frame_x2() const { return m_x2; }
  int16_t frame_y2() const { return m_y2; }
  int16_t frame_z() const { return m_z; }
  int16_t frame_width() const { return m_x2 - m_x1; }
  int16_t frame_height() const { return m_y2 - m_y1; }
  window & owner() { return *m_owner; }
  bool visible() const { return m_visible; }
  void set_frame_pos(int16_t x1, int16_t y1);
  void set_frame_size(int16_t width, int16_t height);
  void set_frame_depth(int16_t z);
  void set_frame(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t z);
  void set_owner(window & n_owner);
  void show();
  void hide();
  void repaint();
  void repaint(int16_t x1, int16_t y1, int16_t x2, int16_t y2);

  virtual void paint(graphics & g) const = 0;

private:
  int16_t m_x1;
  int16_t m_y1;
  int16_t m_x2;
  int16_t m_y2;
  int16_t m_z;
  window * m_owner;
  bool m_visible;
};


#endif // __ELEMENT_H_INCLUDED


