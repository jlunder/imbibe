#ifndef __TASK_MANAGER_HH_INCLUDED
#define __TASK_MANAGER_HH_INCLUDED


class task_manager;


#include "task.hh"
#include "vector.hh"


class task_manager
{
public:
  task_manager();
  void add_task(task & t);
  void remove_task(task & t);
  void run();

private:
  typedef vector < task * > task_p_list;

  bool m_busy;
  task_p_list m_tasks;
  task_p_list m_tasks_to_add;
  task_p_list m_tasks_to_remove;
};


#endif //__TASK_MANAGER_HH_INCLUDED


