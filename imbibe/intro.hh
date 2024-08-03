#ifndef __INTRO_HH_INCLUDED
#define __INTRO_HH_INCLUDED


#include "imbibe.hh"

#include "task.hh"


class intro_task: public task
{
public:
  intro_task(task_manager & n_owner, char const * bin_path): task(n_owner) {}
  bool done() {return true;}

private:
  enum state_enum
  {
    idle
  };

  state_enum m_state;
};


#endif // __INTRO_HH_INCLUDED

