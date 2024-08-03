#ifndef __ELEMENT_HH_INCLUDED
#define __ELEMENT_HH_INCLUDED


#include "imbibe.hh"


class element;


#include "graphics.hh"
#include "window.hh"


class element
{
public:
  element(element const & n_element);
  element(int n_x1, int n_y1, int n_x2, int n_y2, int n_z, window & n_owner, bool n_visible = false);
  int frame_x1() const;
  int frame_y1() const;
  int frame_x2() const;
  int frame_y2() const;
  int frame_z() const;
  int frame_width() const;
  int frame_height() const;
  bool visible() const;
  void set_frame_pos(int x1, int y1);
  void set_frame_size(int width, int height);
  void set_frame_depth(int z);
  void set_frame(int x1, int y1, int x2, int y2, int z);
  window & owner();
  void repaint();
  void repaint(int x1, int y1, int x2, int y2);
  void show();
  void hide();
  virtual void paint(graphics & g) const = 0;

private:
  int m_x1;
  int m_y1;
  int m_x2;
  int m_y2;
  int m_z;
  window & m_owner;
  bool m_visible;
};


//#include "element.ii"


#endif //__ELEMENT_HH_INCLUDED


