#include "imbibe.h"

// #include "timed_task.h"
#include "timed_ta.h"

#include "task.h"
#include "timer.h"


timed_task::timed_task(uint32_t n_duration)
  : m_duration(n_duration), m_time(0) {
}


void timed_task::run() {
  m_time += m_t.delta_ms();
  if(m_time >= m_duration)
  {
    stop();
  }
}


void timed_task::reset() {
  m_t.delta_ms();
  m_time = 0;
}


