#include "imbibe.hh"

#include "timed_task.hh"

#include "task.hh"
#include "timer.hh"


timed_task::timed_task(uint32_t n_duration):
  m_duration(n_duration), m_time(0)
{
}


void timed_task::run()
{
  m_time += m_t.delta_time();
  if(m_time >= m_duration)
  {
    stop();
  }
}


void timed_task::reset()
{
  m_t.delta_time();
  m_time = 0;
}


