#ifndef __TIMER_HH_INCLUDED
#define __TIMER_HH_INCLUDED


class timer
{
public:
  timer();
  unsigned long delta_time(); //in milliseconds

private:
  class hw_timer
  {
  public:
    hw_timer();
    ~hw_timer();
    unsigned long time();

  private:
    static void start_timer();
    static void stop_timer();
    static void (__interrupt pit_handler_bios_slower)();
    static void (__interrupt pit_handler_bios_faster)();
    static void (__interrupt * pit_bios_handler)();
    static unsigned long pit_tick_inc;
    static unsigned long pit_tick_bios_inc;
    static unsigned long pit_tick_count;
    static volatile unsigned long timer_count;
  };

  static hw_timer hw_t;
  unsigned long m_current_time;
};


#endif //__TIMER_HH_INCLUDED


