#ifndef __TASK_HH_INCLUDED
#define __TASK_HH_INCLUDED


class task;


#include "task_manager.hh"


class task
{
public:
  task(task_manager & n_owner);
  virtual void begin();
  virtual void pre_cycle();
  virtual void cycle();
  virtual void post_cycle();
  virtual void end();
  void start();
  void stop();
  task_manager & owner();

private:
  task_manager & m_owner;
};


#endif //__TASK_HH_INCLUDED


