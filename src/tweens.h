#ifndef __TWEENS_H_INCLUDED
#define __TWEENS_H_INCLUDED


#include "imbibe.h"

#include "animation.h"


namespace tweens {
  template<class T>
  class tween_base: public animation {
  public:
    tween_base(T __far * n_target, T n_from, T n_to)
      : m_target(n_target), m_base(n_from), m_delta(n_to - n_from) { }

  protected:
    T __far * m_target;
    T m_base;
    T m_delta;
  };

  template<class T>
  class linear: public tween_base<T> {
  public:
    linear(T __far * n_target, T n_from, T n_to)
      : tween_base<T>(n_target, n_from, n_to) {}

    virtual void update(animation::time_t t_last, animation::time_t t_cur) {
      (void)t_last;
      *this->m_target =
        ((animation::large_time_t)this->m_delta * t_cur
          + animation::t_one / 2) / animation::t_one
        + this->m_base;
    }
  };

  template<class T>
  class ease_in_quad: public tween_base<T> {
  public:
    ease_in_quad(T __far * n_target, T n_from, T n_to)
      : tween_base<T>(n_target, n_from, n_to) {}

    virtual void update(animation::time_t t_last, animation::time_t t_cur) {
      (void)t_last;
      *this->m_target =
        ((animation::large_time_t)this->m_delta * t_cur
          + animation::t_one / 2) / animation::t_one
        + this->m_base;
    }
  };

  template<class T>
  class ease_out_quad: public tween_base<T> {
  public:
    ease_out_quad(T __far * n_target, T n_from, T n_to)
      : tween_base<T>(n_target, n_from, n_to) {}

    virtual void update(animation::time_t t_last, animation::time_t t_cur) {
      (void)t_last;
      *this->m_target =
        ((animation::large_time_t)this->m_delta * t_cur
          + animation::t_one / 2) / animation::t_one
        + this->m_base;
    }
  };

}


class tween {
public:
  tween(): m_duration(0) { }
  virtual ~tween() { }

  virtual void update(animation::time_t delta_time) = 0;

  animation::time_t cur_time() const { return m_last_time; }
  animation::time_t remaining() const { return m_duration - m_last_time; }
  animation::time_t duration() const { return m_duration; }
  bool done() const { return m_last_time >= m_duration; }

protected:
  void aux_reset(animation::time_t n_duration, animation::time_t n_cur_time) {
    assert_margin(n_cur_time, INT16_MAX);
    assert(n_duration >= 0); assert_margin(n_duration, INT16_MAX);
    m_last_time = n_cur_time;
    m_duration = n_duration;
  }

  animation::time_t aux_update(animation::time_t delta_time) {
    assert(delta_time > 0); assert_margin(delta_time, INT16_MAX);

    animation::time_t cur_time = m_last_time + delta_time;

    if (cur_time >= m_duration) {
      m_last_time = m_duration;
      return 1;
    }

    m_last_time = cur_time;
    return cur_time;
  }

private:
  animation::time_t m_last_time;
  animation::time_t m_duration;
};


class timer_tween: public tween {
public:
  void reset(animation::time_t n_duration, animation::time_t n_delay = 0) {
    aux_reset(n_duration, n_delay);
  }

  virtual void update(animation::time_t delta_time) {
    aux_update(delta_time);
  }
};


template<class T, class TEasing>
class value_tween: public tween {
public:
  value_tween(): m_from(0), m_delta(0), m_value(0) { }

  void reset(T n_from, T n_to, animation::time_t n_duration,
      animation::time_t n_delay = 0) {
    aux_reset(n_duration, -n_delay);
    m_from = n_from;
    m_delta = n_to - n_from;
    animation::time_t cur = cur_time();
    m_value = (cur <= 0) ? n_from
      : ((cur >= n_duration) ? n_to
        : TEasing::compute_value(cur, duration(), m_from, m_delta));
  }

  void reset_from_value(T n_from, T n_to, animation::time_t n_duration,
      T value) {
    m_from = n_from;
    m_delta = n_to - n_from;
    if (((m_delta > 0) && (value < n_from))
        || ((m_delta < 0) && (value > n_from))) {
      m_value = n_from;
      aux_reset(n_duration, 0);
    } else if (((m_delta > 0) && (value > n_to))
        || ((m_delta < 0) && (value < n_to))) {
      m_value = n_to;
      aux_reset(n_duration, n_duration);
    } else {
      m_value = value;
      aux_reset(n_duration,
        TEasing::compute_time(value, n_from, n_to, n_duration));
    }
    m_value = done() ? n_to : n_from;
  }

  void update(animation::time_t delta_time) {
    if (done()) {
      assert(m_value == m_from + m_delta);
      return;
    }

    animation::time_t cur_time = aux_update(delta_time);
    if(cur_time < 0) {
      // do nothing, we wait for delay to pass
      assert(!done());
      assert(m_value == m_from);
    } else if (cur_time < duration()) {
      m_value =
        TEasing::compute_value(cur_time, duration(), m_from, m_delta);
    } else {
      m_value = m_from + m_delta;
    }
  }

  T value() const { return m_value; }

private:
  T m_from;
  T m_delta;
  T m_value;
};


template<class T>
class linear_easing {
public:
  static T compute_value(animation::time_t cur_time,
      animation::time_t duration, T from, T delta) {
    assert(cur_time >= 0); assert(cur_time <= duration);
    return from + (delta * (animation::large_time_t)cur_time) / duration;
  }

  static animation::time_t compute_time(T value, T from, T delta,
      animation::time_t duration) {
    assert(value >= from); assert(value <= from + delta);
    return ((value - from) * (animation::large_time_t)duration) / delta;
  }
};


template<class T>
class linear_tween: public value_tween<T, linear_easing<T> > {
};


// template<class T>
// class quadratic_in_easing {
// public:
//   static T compute_value(animation::time_t cur_time,
//       animation::time_t duration, T from, T delta) {
//     return from + (delta * (animation::large_time_t)cur_time) / duration;
//   }
// };


// template<class T>
// class quadratic_out_easing {
// public:
//   static T compute_value(animation::time_t cur_time,
//       animation::time_t duration, T from, T delta) {
//     return from + (delta * (animation::large_time_t)cur_time) / duration;
//   }
// };


#endif // __TWEENS_H_INCLUDED


