#ifndef __TBM_ELEMENT_H_INCLUDED
#define __TBM_ELEMENT_H_INCLUDED

#include "imbibe.h"

#include "element.h"
#include "tbm.h"

class graphics;

class tbm_element : public element {
public:
  tbm_element();
  virtual ~tbm_element() {}
  void set_tbm(tbm const &n_t);
  void set_fade(uint8_t n_fade);
  virtual void paint(graphics *g);

private:
  tbm m_t;
  uint8_t m_fade;
};

#endif // __TBM_ELEMENT_H_INCLUDED
