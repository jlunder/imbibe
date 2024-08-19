#ifndef __BITMAP_ELEMENT_H_INCLUDED
#define __BITMAP_ELEMENT_H_INCLUDED


#include "imbibe.h"

#include "bitmap.h"
#include "element.h"
#include "graphics.h"


class bitmap_element: public element
{
public:
  bitmap_element();
  virtual ~bitmap_element();
  void set_b(bitmap * n_b);
  bitmap & b() { return *m_b; }
  bitmap const & b() const { return *m_b; }
  virtual void paint(graphics & g);

private:
  bitmap * m_b;
};


#endif // __BITMAP_ELEMENT_H_INCLUDED


