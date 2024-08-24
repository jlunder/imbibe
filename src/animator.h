#ifndef __ANIMATOR_H_INCLUDED
#define __ANIMATOR_H_INCLUDED


#include "imbibe.h"

#include "animation.h"
#include "map.h"
#include "task.h"


class animator {
public:
  typedef uint8_t key_t;

  ~animator();

  void animate(animation::time_t delta_time);
  void play(key_t key, animation::time_t n_delay, animation::time_t n_length,
    animation & n_anim);
  void clear(key_t key);
  void clear_all();

  animation::time_t last_time() const { return m_last_time; }

private:
  struct animation_state {
    animation * m_anim;
    animation::time_t m_cur_time;
    animation::time_t m_length;
    animation::time_t m_t_last;
  };

  typedef map<key_t, animation_state> active_map;

  void * m_target;
  animation::time_t m_last_time;
  active_map m_active;
};


#endif // __ANIMATOR_H_INCLUDED


