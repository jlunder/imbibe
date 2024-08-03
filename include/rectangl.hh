#ifndef __RECTANGLE_ELEMENT_HH_INCLUDED
#define __RECTANGLE_ELEMENT_HH_INCLUDED


#include "imbibe.hh"

#include "graphics.hh"
#include "element.hh"
#include "window.hh"
#include "pixel.hh"


class rectangle_element: public element
{
public:
  rectangle_element(int n_x1, int n_y1, int n_x2, int n_y2, int n_z, window & n_owner, pixel n_p);
  virtual void paint(graphics & g) const;

private:
  pixel m_p;
};


#endif //__RECTANGLE_ELEMENT_HH_INCLUDED


