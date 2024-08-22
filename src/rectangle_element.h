#ifndef __RECTANGLE_ELEMENT_H_INCLUDED
#define __RECTANGLE_ELEMENT_H_INCLUDED


#include "imbibe.h"

#include "graphics.h"
#include "element.h"
#include "termviz.h"
#include "window.h"


class rectangle_element: public element
{
public:
  rectangle_element(int n_x1, int n_y1, int n_x2, int n_y2, int n_z,
    window & n_owner, termel_t n_p);
  virtual void paint(graphics & g);

private:
  termel_t m_p;
};


#endif // __RECTANGLE_ELEMENT_H_INCLUDED

