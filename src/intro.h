#ifndef __INTRO_H_INCLUDED
#define __INTRO_H_INCLUDED


#include "imbibe.h"

#include "task.h"


class intro_task: public task
{
public:
  intro_task(char const * bin_path) {}
  bool done() {return true;}

private:
  enum state_enum
  {
    idle
  };

  state_enum m_state;
};


#endif // __INTRO_H_INCLUDED

