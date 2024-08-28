#ifndef __TBM_ELEMENT_H_INCLUDED
#define __TBM_ELEMENT_H_INCLUDED

#include "imbibe.h"

#include "element.h"
#include "unpacker.h"

class graphics;

class tbm_element : public element {
public:
  tbm_element();
  virtual ~tbm_element();
  void set_tbm(unpacker const &n_tbm);
  void set_fade(uint8_t n_fade);
  virtual void paint(graphics &g);

private:
  unpacker m_tbm;
  uint8_t m_fade;
};

#endif // __TBM_ELEMENT_H_INCLUDED
