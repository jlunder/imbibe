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


class main_task: public task {
public:
  main_task();
  ~main_task();
  bool done() { return m_state == st_done; }
  virtual void run();
  virtual void idle();

  void run_loop();

private:
  enum state_t {
    st_loading,
    st_waiting,
    st_done
  };

  class key_handler_thunk: public key_handler {
  public:
    key_handler_thunk(main_task & n_target): m_target(n_target) { }
    main_task & m_target;
    virtual bool handle_key(uint16_t key);
  };

  state_t m_state;
  text_window m_win;
  bitmap_element m_canvas;
  key_handler_thunk m_key_thunk;

  bool handle_key(uint16_t key);
};


#endif // __MAIN_TASK_H_INCLUDED

