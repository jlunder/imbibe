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
  rectangle_element();
  virtual void paint(graphics & g);

  void set_brush(termel_t n_brush);

private:
  termel_t m_brush;
};


#endif // __RECTANGLE_ELEMENT_H_INCLUDED


