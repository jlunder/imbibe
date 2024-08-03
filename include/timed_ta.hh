#ifndef __TIMED_TASK_HH_INCLUDED
#define __TIMED_TASK_HH_INCLUDED


class timed_task;


#include "task.hh"
#include "timer.hh"


class timed_task: public task
{
public:
  timed_task(task_manager & n_owner, unsigned long n_duration);
  virtual void begin();
  virtual void pre_cycle();
  unsigned long duration();
  unsigned long time();

private:
  unsigned long m_duration;
  unsigned long m_time;
  timer m_t;
};


#endif //__TIMED_TASK_HH_INCLUDED


