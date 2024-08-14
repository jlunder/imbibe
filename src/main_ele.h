#ifndef __MAIN_ELEMENT_H_INCLUDED
#define __MAIN_ELEMENT_H_INCLUDED


#include "imbibe.h"

// #include "window_element.h"
#include "window_e.h"


class main_element : public window_element {
public:
  main_element();
  virtual ~main_element();

  void animate(uint32_t delta_ms);

private:
  window_element m_frame;
  window_element m_scroll;
};


#endif // __MAIN_ELEMENT_H_INCLUDED

