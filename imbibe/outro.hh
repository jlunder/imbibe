#ifndef __OUTRO_HH_INCLUDED
#define __OUTRO_HH_INCLUDED


#include "imbibe.hh"

#include "task.hh"


class outro_task: public task
{
public:
  outro_task(task_manager & n_owner, char const * bin_path): task(n_owner) {}
  bool done() {return true;}

private:
  enum state_enum
  {
    idle
  };

  state_enum m_state;
};


#endif // __OUTRO_HH_INCLUDED

