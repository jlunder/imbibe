#include "cplusplus.hh"

#include "timed_task.hh"

#include "task.hh"
#include "timer.hh"

#include "timed_task.ii"

#include "task.ii"
#include "timer.ii"


timed_task::timed_task(task_manager & n_owner, unsigned long n_duration):
  task(n_owner), m_duration(n_duration)
{
}


void timed_task::begin()
{
  m_t.delta_time();
  m_time = 0;
}


void timed_task::pre_cycle()
{
  m_time += m_t.delta_time();
  if(m_time >= m_duration)
  {
    stop();
  }
}


