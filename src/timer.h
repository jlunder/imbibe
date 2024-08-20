#ifndef __TIMER_H_INCLUDED
#define __TIMER_H_INCLUDED


#include "imbibe.h"


class timer
{
public:
  timer(): m_last_ms(now()) { }

  //in milliseconds
  uint32_t reset_ms() {
    uint32_t new_ms = now();
    uint32_t result = new_ms - m_last_ms;

    m_last_ms = new_ms;
    return result;
  }

  uint32_t delta_ms() {
    uint32_t new_ms = now();
    return new_ms - m_last_ms;
  }

  bool reset_if_elapsed(uint32_t period_ms) {
    uint32_t new_ms = now();
    if ((new_ms - m_last_ms) < period_ms) {
      return false;
    } else {
      m_last_ms = new_ms;
      return true;
    }
  }

  //in milliseconds
  static uint32_t now();

  static void setup();
  static void teardown();

private:
  uint32_t m_last_ms;
};


#endif // __TIMER_H_INCLUDED


