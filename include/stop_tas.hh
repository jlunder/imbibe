#ifndef __STOP_TASK_HH_INCLUDED
#define __STOP_TASK_HH_INCLUDED


#include "task.hh"
#include "task_manager.hh"
#include "vector.hh"


class stop_task: public task
{
public:
  stop_task(task_manager & n_owner);
  virtual void end();
  void add_task(task & t);
  void remove_task(task & t);

private:
  typedef vector < task * > task_p_list;

  task_p_list m_tasks;
};


#endif //__STOP_TASK_HH_INCLUDED


