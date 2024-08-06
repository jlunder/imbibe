#ifndef __MAIN_TASK_H_INCLUDED
#define __MAIN_TASK_H_INCLUDED


#include "imbibe.h"

// #include "bitmap_element.h"
#include "bitmap_e.h"
// #include "key_handler.h"
#include "key_hand.h"
#include "task.h"
// #include "text_window.h"
#include "text_win.h"


class main_task: public task, public key_handler {
public:
  main_task();
  ~main_task();
  bool done() { return m_state == st_done; }
  virtual void run();
  virtual void idle();
  virtual bool handle_key(uint16_t key);

  void run_loop();

private:
  enum state_t {
    st_loading,
    st_waiting,
    st_done
  };

  state_t m_state;
  text_window m_win;
  bitmap_element m_canvas;
};


#endif // __MAIN_TASK_H_INCLUDED

