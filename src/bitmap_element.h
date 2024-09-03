#ifndef __BITMAP_ELEMENT_H_INCLUDED
#define __BITMAP_ELEMENT_H_INCLUDED

#include "imbibe.h"

#include "element.h"
#include "immutable.h"

class bitmap;
class graphics;

class bitmap_element : public element {
public:
  bitmap_element();
  virtual ~bitmap_element();
  void set_b(im_ptr<bitmap> n_b);
  void set_fade(uint8_t n_fade);
  im_ptr<bitmap> b() const { return m_b; }
  virtual void paint(graphics *g);

private:
  im_ptr<bitmap> m_b;
  uint8_t m_fade;
};

#endif // __BITMAP_ELEMENT_H_INCLUDED
