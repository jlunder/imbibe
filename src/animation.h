#ifndef __ANIMATION_H_INCLUDED
#define __ANIMATION_H_INCLUDED


#include "imbibe.h"


class animation {
public:
  typedef int16_t time_t;
  typedef int32_t large_time_t;

  static const time_t t_one = 1 << 12;

  virtual ~animation() { }

  virtual void update(time_t last_t, time_t t) = 0;
};


#endif // __ANIMATION_H_INCLUDED


