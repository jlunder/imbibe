#ifndef __BITMAP_ELEMENT_HH_INCLUDED
#define __BITMAP_ELEMENT_HH_INCLUDED


#include "imbibe.hh"

#include "bitmap.hh"
#include "element.hh"
#include "graphics.hh"
#include "window.hh"


class bitmap_element: public element
{
public:
  bitmap_element(int n_x1, int n_y1, int n_x2, int n_y2, int n_z, window & n_owner, bitmap * n_b);
  virtual ~bitmap_element();
  void b(bitmap * n_b);
  bitmap & b();
  bitmap const & b() const;
  virtual void paint(graphics & g) const;

private:
  bitmap * m_b;
};


#endif //__BITMAP_ELEMENT_HH_INCLUDED


