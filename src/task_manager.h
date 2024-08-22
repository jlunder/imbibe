#ifndef __TASK_MANAGER_H_INCLUDED
#define __TASK_MANAGER_H_INCLUDED


#include "imbibe.h"

#include "reclaim.h"
#include "vector.h"


class task;


class action
{
public:
  typedef void (*reclaim_func_t)(action __far * p);

  virtual ~action() {}
  virtual void operator()() {}
  virtual reclaim_func_t reclaimer() const { return default_reclaim; }

private:
  static void default_reclaim(action __far * p);
};


class reusable_action : public action
{
public:
  virtual reclaim_func_t reclaimer() const { return do_nothing_reclaim; }

private:
  static void do_nothing_reclaim(action __far * p);
};


class task_manager
{
public:
  static void add_task(task & t);
  static void remove_task(task & t);
  static void request_run_task(task & t);
  static void defer_action(action * action);
  static bool poll();
  static bool run();
  static void idle();

private:
  typedef vector<task *> task_p_list;
  typedef vector<action *> action_p_list;

  static bool s_busy;
  static task_p_list s_tasks;
  static task_p_list s_tasks_to_poll;
  static task_p_list s_tasks_to_run;
  static task_p_list s_tasks_to_idle;
  static action_p_list s_deferred_actions;
};


#endif // __TASK_MANAGER_H_INCLUDED


