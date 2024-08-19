#ifndef __RENDER_TEST_TASK_H_INCLUDED
#define __RENDER_TEST_TASK_H_INCLUDED


#include "imbibe.h"

#include "bitmap_element.h"
#include "task.h"
#include "termviz.h"
#include "text_window.h"
#include "timer.h"
#include "window_element.h"


class render_test_task: public task {
public:
  render_test_task();
  ~render_test_task();
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
    repaint_element(attribute_t fill): m_fill(fill) { }
    virtual void paint(graphics & g);
  private:
    attribute_t m_fill;
    static uint8_t m_counter;
  };

  class text_element: public element {
  public:
    text_element(termel_t fill, attribute_t text, char const * message):
      m_fill(fill), m_text(text), m_message(message) { }
    virtual void paint(graphics & g);
  private:
    termel_t m_fill;
    attribute_t m_text;
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
};


#endif // __RENDER_TEST_TASK_H_INCLUDED

