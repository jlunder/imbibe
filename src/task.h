#ifndef __TASK_H_INCLUDED
#define __TASK_H_INCLUDED


#include "imbibe.h"

#include "task_manager.h"


class task
{
public:
  typedef void (*reclaim_func_t)(task __far * p);

  virtual ~task() {}
  virtual void poll() {}
  virtual void run() {}
  virtual void idle() {}
  virtual reclaim_func_t reclaimer() const { return do_nothing_reclaim; }

  void start(bool run_immediately = true) {
    task_manager::add_task(*this);
    if(run_immediately) {
      task_manager::request_run_task(*this);
    }
  }

  void request_run()
    { task_manager::request_run_task(*this); }

  void stop() { task_manager::remove_task(*this); }

private:
  static void do_nothing_reclaim(task __far * p);
};


#endif // __TASK_H_INCLUDED


