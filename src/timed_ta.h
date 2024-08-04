#ifndef __TIMED_TASK_HH_INCLUDED
#define __TIMED_TASK_HH_INCLUDED


#include "task.h"
#include "timer.h"


class timed_task: public task
{
public:
  timed_task(uint32_t n_duration);
  virtual void run();
  void reset();
  uint32_t duration() { return m_duration; }
  uint32_t time() { return m_time; }

private:
  uint32_t m_duration;
  uint32_t m_time;
  timer m_t;
};




#endif //__TIMED_TASK_HH_INCLUDED


