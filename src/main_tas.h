#ifndef __MAIN_TASK_H_INCLUDED
#define __MAIN_TASK_H_INCLUDED


#include "imbibe.h"

// #include "bitmap_element.h"
#include "bitmap_e.h"
#include "color.h"
// #include "main_element.h"
#include "main_ele.h"
#include "task.h"
// #include "text_window.h"
#include "text_win.h"
#include "timer.h"


class main_task: public task {
public:
  main_task();
  ~main_task();
  bool done() { return m_state == st_done; }
  virtual void poll();
  virtual void run();
  virtual void idle();

  void run_loop();

private:
  enum state_t {
    st_loading,
    st_waiting,
    st_done
  };

  timer m_frame_timer;
  state_t m_state;
  uint32_t m_anim_time;

  text_window m_win;
  main_element m_main;
};


#endif // __MAIN_TASK_H_INCLUDED

