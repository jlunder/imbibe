#ifndef __QUIT_PROMPT_ELEMENT_H_INCLUDED
#define __QUIT_PROMPT_ELEMENT_H_INCLUDED

#include "imbibe.h"

#include "screen_element.h"
#include "tbm.h"

class quit_element : public screen_element {
public:
  quit_element();
  virtual ~quit_element() {}

  virtual void layout(coord_t window_width, coord_t window_height);
  virtual void poll();
  virtual bool handle_key(key_code_t key);
  virtual void animate(anim_time_t delta_mu);
  virtual bool active() const;
  virtual bool opaque() const { return false; }
  virtual void paint(graphics *g);

  void prompt_quit();

private:
  tbm m_quit_tbm;
  bool m_active;
};

#endif // __QUIT_PROMPT_ELEMENT_H_INCLUDED
