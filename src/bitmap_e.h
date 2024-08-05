#ifndef __BITMAP_ELEMENT_H_INCLUDED
#define __BITMAP_ELEMENT_H_INCLUDED


#include "imbibe.h"

#include "bitmap.h"
#include "element.h"
#include "graphics.h"
#include "window.h"


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


inline void bitmap_element::b(bitmap * n_b)
{
  m_b = n_b;
  repaint();
}


inline bitmap & bitmap_element::b()
{
  return *m_b;
}


inline bitmap const & bitmap_element::b() const
{
  return *m_b;
}


#endif // __BITMAP_ELEMENT_H_INCLUDED


