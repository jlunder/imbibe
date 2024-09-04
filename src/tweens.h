#ifndef __TWEENS_H_INCLUDED
#define __TWEENS_H_INCLUDED

#include "imbibe.h"

class tween {
public:
  tween() : m_duration(0) {}
  virtual ~tween() {}

  virtual void update(anim_time_t delta_time) = 0;

  void finish() { update(remaining()); }

  anim_time_t cur_time() const { return m_last_time; }
  anim_time_t remaining() const { return m_duration - m_last_time; }
  anim_time_t duration() const { return m_duration; }
  bool done() const { return m_last_time >= m_duration; }

protected:
  void aux_reset(anim_time_t n_duration, anim_time_t n_cur_time) {
    assert(n_cur_time > INT16_MIN / 2);
    assert(n_cur_time < INT16_MAX / 2);
    assert(n_duration >= 0);
    assert(n_duration < INT16_MAX / 2);
    m_last_time = n_cur_time;
    m_duration = n_duration;
  }

  anim_time_t aux_update(anim_time_t delta_time) {
    assert(delta_time > 0);
    assert(delta_time < INT16_MAX / 2);

    anim_time_t cur_time = m_last_time + delta_time;

    if (cur_time >= m_duration) {
      m_last_time = m_duration;
      return m_duration;
    }

    m_last_time = cur_time;
    return cur_time;
  }

private:
  anim_time_t m_last_time;
  anim_time_t m_duration;
};

class countdown_tween : public tween {
public:
  void reset(anim_time_t n_duration, anim_time_t n_delay = 0) {
    aux_reset(n_duration, n_delay);
  }

  virtual void update(anim_time_t delta_time) {
    if (delta_time > 0) {
      aux_update(delta_time);
    }
  }
};

template <class T, class TEasing> class value_tween : public tween {
public:
  value_tween() : m_from(0), m_delta(0), m_value(0) {}

  void reset(T n_from, T n_to, anim_time_t n_duration,
             anim_time_t n_delay = 0) {
    aux_reset(n_duration, -n_delay);
    m_from = n_from;
    m_delta = n_to - n_from;
    anim_time_t cur = cur_time();
    if (cur >= n_duration) {
      m_value = n_to;
    } else if (cur <= 0) {
      m_value = n_from;
    } else {
      m_value = TEasing::compute_value(cur, duration(), m_from, m_delta);
    }
  }

  void reset_from_value(T n_from, T n_to, anim_time_t n_duration, T value) {
    m_from = n_from;
    m_delta = n_to - n_from;
    if (((m_delta > 0) && (value < n_from)) ||
        ((m_delta < 0) && (value > n_from))) {
      m_value = n_from;
      aux_reset(n_duration, 0);
    } else if (((m_delta > 0) && (value > n_to)) ||
               ((m_delta < 0) && (value < n_to))) {
      m_value = n_to;
      aux_reset(n_duration, n_duration);
    } else {
      m_value = value;
      aux_reset(n_duration,
                TEasing::compute_time(value, n_from, n_to, n_duration));
    }
    m_value = done() ? n_to : n_from;
  }

  void update(anim_time_t delta_time) {
    if (done()) {
      assert(m_value == m_from + m_delta);
      return;
    }
    if (delta_time == 0) {
      assert((cur_time() < 0) ||
             (m_value ==
              TEasing::compute_value(cur_time(), duration(), m_from, m_delta)));
      return;
    }

    anim_time_t cur_time = aux_update(delta_time);
    if (cur_time < 0) {
      // do nothing, we wait for delay to pass
      assert(!done());
      assert(m_value == m_from);
    } else if (cur_time < duration()) {
      m_value = TEasing::compute_value(cur_time, duration(), m_from, m_delta);
    } else {
      m_value = m_from + m_delta;
    }
  }

  T update_delta(anim_time_t delta_time) {
    T last_value = value();
    update(delta_time);
    return value() - last_value();
  }

  T value() const { return m_value; }

private:
  T m_from;
  T m_delta;
  T m_value;
};

template <class T> class linear_easing {
public:
  static T compute_value(anim_time_t cur_time, anim_time_t duration, T from,
                         T delta) {
    assert(cur_time >= 0);
    assert(cur_time <= duration);
    return (T)(from + (delta * (large_anim_time_t)cur_time) / duration);
  }

  static anim_time_t compute_time(T value, T from, T delta,
                                  anim_time_t duration) {
    assert(value >= from);
    assert(value <= from + delta);
    return (anim_time_t)(((value - from) * (large_anim_time_t)duration) /
                         delta);
  }
};

// template<class T>
// class quadratic_in_easing {
// public:
// static T compute_value(anim_time_t cur_time,
//     anim_time_t duration, T from, T delta) {
//   assert(cur_time >= 0); assert(cur_time <= duration);
//   return
//     (T)(from + (delta * (large_anim_time_t)cur_time) / duration);
// }

// static anim_time_t compute_time(T value, T from, T delta,
//     anim_time_t duration) {
//   assert(value >= from); assert(value <= from + delta);
//   return (anim_time_t)(
//     ((value - from) * (large_anim_time_t)duration) / delta);
// }
// };

template <class T>
class linear_tween : public value_tween<T, linear_easing<T> > {};

// template<class T>
// class quadratic_in_tween: public value_tween<T, quadratic_in_easing<T> > {
// };

#endif // __TWEENS_H_INCLUDED
