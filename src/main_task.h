#ifndef __MAIN_TASK_H_INCLUDED
#define __MAIN_TASK_H_INCLUDED


#include "imbibe.h"

#include "main_element.h"
#include "task.h"
#include "text_window.h"
#include "timer.h"


class main_task: public task {
public:
  main_task();
  ~main_task();
  virtual void poll();
  virtual void run();
  virtual void idle();

  void run_loop();

  static void exit();

private:
  timer m_frame_timer;

  text_window m_win;
  main_element m_main;

  bool m_exit_requested;
};


extern main_task main_task_instance;


#endif // __MAIN_TASK_H_INCLUDED

