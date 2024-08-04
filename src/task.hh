#ifndef __TASK_HH_INCLUDED
#define __TASK_HH_INCLUDED


#include "imbibe.hh"

#include "reclaim.hh"
// #include "task_manager.hh"
#include "task_man.hh"


class task
{
public:
  virtual ~task() {}
  virtual void run() {}
  virtual void idle() {}
  virtual reclaim<task> & reclaimer() const { return s_do_nothing_reclaim; }

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
  static do_nothing_reclaim<task> s_do_nothing_reclaim;
};


#endif //__TASK_HH_INCLUDED


