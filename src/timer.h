#ifndef __TIMER_H_INCLUDED
#define __TIMER_H_INCLUDED


#include "imbibe.h"


class timer
{
public:
  timer(): m_last_ms(now()) { }

  uint32_t read_ms() {
    uint32_t new_ms = now();
    uint32_t delta_ms = new_ms - m_last_ms;
    m_last_ms = new_ms;
    return delta_ms;
  }

  uint32_t read_at_least_ms(uint32_t min_period_ms) {
    uint32_t new_ms = now();
    uint32_t delta_ms = new_ms - m_last_ms;
    if (delta_ms >= min_period_ms) {
      m_last_ms = new_ms;
      return delta_ms;
    } else {
      return 0;
    }
  }

  uint32_t read_exact_periods(uint32_t min_period_ms) {
    assert(min_period_ms > 0);

    uint32_t new_ms = now();
    uint32_t delta_ms = new_ms - m_last_ms;
    if (delta_ms >= min_period_ms * 2) {
      ldiv_t result = ldiv(new_ms, min_period_ms);
      m_last_ms = new_ms - (uint32_t)result.rem;
      return (uint32_t)result.quot;
    } else if (delta_ms >= min_period_ms) {
      return 1;
    } else {
      return 0;
    }
  }

  uint32_t read_exact_ms(uint32_t min_period_ms) {
    assert(min_period_ms > 0);

    uint32_t new_ms = now();
    uint32_t delta_ms = new_ms - m_last_ms;
    if (delta_ms >= min_period_ms * 2) {
      ldiv_t result = ldiv(new_ms, min_period_ms);
      m_last_ms = new_ms - (uint32_t)result.rem;
      return delta_ms - (uint32_t)result.rem;
    } else if (delta_ms >= min_period_ms) {
      return min_period_ms;
    } else {
      return 0;
    }
  }

  uint32_t peek_ms() {
    uint32_t new_ms = now();
    return new_ms - m_last_ms;
  }

  //in milliseconds
  static uint32_t now();

  static void setup();
  static void teardown();

private:
  uint32_t m_last_ms;
};


#endif // __TIMER_H_INCLUDED


