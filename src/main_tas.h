#ifndef __MAIN_TASK_H_INCLUDED
#define __MAIN_TASK_H_INCLUDED


#include "imbibe.h"

// #include "bitmap_element.h"
#include "bitmap_e.h"
#include "color.h"
// #include "key_handler.h"
#include "key_hand.h"
#include "task.h"
// #include "text_window.h"
#include "text_win.h"
#include "timer.h"
// #include "window_element.h"
#include "window_e.h"


class main_task: public task, private key_handler {
public:
  main_task();
  ~main_task();
  bool done() { return m_state == st_done; }
  virtual void poll();
  virtual void run();
  virtual void idle();

  void animate(uint32_t delta_ms);

  void run_loop();

private:
  enum state_t {
    st_loading,
    st_waiting,
    st_done
  };

  class repaint_element: public element {
  public:
    repaint_element(color fill): m_fill(fill) { }
    virtual void paint(graphics & g);
  private:
    color m_fill;
    static uint8_t m_counter;
  };

  class text_element: public element {
  public:
    text_element(pixel fill, color text, char const * message):
      m_fill(fill), m_text(text), m_message(message) { }
    virtual void paint(graphics & g);
  private:
    pixel m_fill;
    color m_text;
    char const * m_message;
  };

  timer m_frame_timer;
  state_t m_state;
  text_window m_win;
  window_element m_frame;

  uint32_t m_anim_time;
  repaint_element m_background;
  window_element m_clipper;
  repaint_element m_clip_background;
  bitmap_element m_orbit1;
  text_element m_orbit2;

  virtual bool handle_key(uint16_t key);
};


#endif // __MAIN_TASK_H_INCLUDED

