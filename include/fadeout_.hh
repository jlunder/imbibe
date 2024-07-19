#ifndef __FADEOUT_TASK_HH_INCLUDED
#define __FADEOUT_TASK_HH_INCLUDED


#include "bitmap.hh"
#include "bitmap_element.hh"
#include "timed_task.hh"
#include "task_manager.hh"


class fadeout_task: public timed_task
{
public:
  fadeout_task(task_manager & n_owner, unsigned long n_duration, bitmap_element & n_e, bitmap const & n_b);
  virtual void begin();
  virtual void cycle();
  virtual void end();

private:
  bitmap_element & m_e;
  bitmap const & m_b;
  unsigned short m_last_mask;
};


#endif //__FADEOUT_TASK_HH_INCLUDED


