#ifndef __OUTRO_H_INCLUDED
#define __OUTRO_H_INCLUDED


#include "imbibe.h"

#include "task.h"


class outro_task: public task
{
public:
  outro_task(char const * bin_path) {}
  bool done() {return true;}

private:
  enum state_enum
  {
    idle
  };

  state_enum m_state;
};


#endif // __OUTRO_H_INCLUDED

