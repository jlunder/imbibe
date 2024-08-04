#ifndef __TASK_MANAGER_HH_INCLUDED
#define __TASK_MANAGER_HH_INCLUDED


#include "imbibe.hh"

#include "reclaim.hh"
#include "vector.hh"


class task;


class action
{
public:
  virtual ~action() {}
  virtual void operator()() {}
  virtual reclaim<action> & reclaimer() const { return s_default_reclaim; }

private:
  static delete_reclaim<action> s_default_reclaim;
};


class reusable_action : public action
{
public:
  virtual reclaim<action> & reclaimer() const { return s_do_nothing_reclaim; }

private:
  static do_nothing_reclaim<action> s_do_nothing_reclaim;
};


class task_manager
{
public:
  task_manager();
  static void add_task(task & t);
  static void remove_task(task & t);
  static void request_run_task(task & t);
  static void defer_action(action * action);
  static bool run();
  static void idle();

private:
  typedef vector<task *> task_p_list;
  typedef vector<action *> action_p_list;

  static bool s_busy;
  static task_p_list s_tasks;
  static task_p_list s_tasks_to_run;
  static action_p_list s_deferred_actions;
  static task_p_list s_tasks_to_reclaim;
};


#endif //__TASK_MANAGER_HH_INCLUDED


