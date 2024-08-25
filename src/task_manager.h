#ifndef __TASK_MANAGER_H_INCLUDED
#define __TASK_MANAGER_H_INCLUDED


#include "imbibe.h"


class task;


class action {
public:
  typedef void (*reclaim_func_t)(action __far * p);

  virtual ~action() {}
  virtual void operator()() {}
  virtual reclaim_func_t reclaimer() const { return default_reclaim; }

private:
  static void default_reclaim(action __far * p);
};


class reusable_action : public action {
public:
  virtual reclaim_func_t reclaimer() const { return do_nothing_reclaim; }

private:
  static void do_nothing_reclaim(action __far * p);
};


namespace task_manager {
  extern void add_task(task & t);
  extern void remove_task(task & t);
  extern void request_run_task(task & t);
  extern void defer_action(action * action);
  extern bool poll();
  extern bool run();
  extern void idle();
}


#endif // __TASK_MANAGER_H_INCLUDED


