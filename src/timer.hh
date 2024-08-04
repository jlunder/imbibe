#ifndef __TIMER_HH_INCLUDED
#define __TIMER_HH_INCLUDED


#include "imbibe.hh"


class timer
{
public:
  timer();
  uint32_t delta_time(); //in milliseconds

private:
  class hw_timer
  {
  public:
    hw_timer();
    ~hw_timer();
    uint32_t time();

  private:
    static void start_timer();
    static void stop_timer();
    static void (__interrupt pit_handler_bios_slower)();
    static void (__interrupt pit_handler_bios_faster)();
    static void (__interrupt * pit_bios_handler)();
    static uint32_t pit_tick_inc;
    static uint32_t pit_tick_bios_inc;
    static uint32_t pit_tick_count;
    static volatile uint32_t timer_count;
  };

  static hw_timer hw_t;
  uint32_t m_current_time;
};


inline uint32_t timer::delta_time()
{
  uint32_t new_time = hw_t.time();
  uint32_t result = new_time - m_current_time;

  m_current_time = new_time;
  return result;
}


inline uint32_t timer::hw_timer::time()
{
  return timer_count;
}


#endif //__TIMER_HH_INCLUDED


