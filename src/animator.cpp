#include "imbibe.h"

#include "animator.h"

#include "animation.h"


animator::~animator() {
  clear_all();
}


void animator::animate(animation::time_t delta_time) {
  assert(delta_time > 0); assert(delta_time < animation::t_one);
  for (active_map::iterator i = m_active.begin(); i != m_active.end(); ) {
    animation_state & st = i->ref;
    assert(st.m_anim); assert(st.m_length >= 0); assert(st.m_t_last >= 0);

    if (st.m_cur_time < -delta_time) {
      // delay not totally elapsed -- update and continue
      st.m_cur_time += delta_time;
      ++i;
    } else {
      // animation playing -- finished animations should be removed
      // immediately, so assert we have time left
      assert(st.m_cur_time < st.m_length);

      // update time
      st.m_cur_time = st.m_cur_time + delta_time;

      animation::time_t t_cur;
      // we just asserted this, but then we modified m_cur_time
      if (st.m_cur_time < st.m_length) {
        // we know m_length should be > 0 because if it was 0, we would
        // still be in the delay period and the test at the beginning of the
        // loop should have kept us from reaching this point
        assert (st.m_length > 0);
        // (context for the above is we're preventing divide by 0 here)
        t_cur = (animation::large_time_t)st.m_cur_time * animation::t_one
          / st.m_length;
      } else {
        // if the animation is at or past end, just clamp to t=1
        t_cur = animation::t_one;
      }

      st.m_anim->update(st.m_t_last, t_cur);

      if (t_cur == animation::t_one) {
        // animation is finished
        i = m_active.erase(i);
      } else {
        // else, not finished, update t_last
        st.m_t_last = t_cur;
        ++i;
      }
    }
  }
}


void animator::play(key_t key, animation::time_t n_delay,
    animation::time_t n_length, animation & n_anim) {
  assert(n_delay >= 0); //assert(n_anim);
  clear(key);
  animation_state st = {
    .m_anim = &n_anim,
    .m_cur_time = (animation::time_t)-n_delay,
    .m_length = n_length,
    .m_t_last = 0,
  };
  m_active.insert(active_map::value_type(key, st));
}


void animator::clear(key_t key) {
  active_map::iterator from = m_active.lower_bound(key);
  active_map::iterator to = m_active.upper_bound(key);
  for (active_map::iterator i = from; i != to; ++i) {
    delete i->ref.m_anim;
  }
  m_active.erase(from, to);
}


void animator::clear_all() {
  for (active_map::iterator i = m_active.begin(); i != m_active.end(); ++i) {
    delete i->ref.m_anim;
  }
  m_active.clear();
}


