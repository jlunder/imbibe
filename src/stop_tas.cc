#include "cplusplus.hh"

#include "stop_task.hh"

#include "task.hh"
#include "task_manager.hh"
#include "vector.hh"

#include "stop_task.ii"

#include "task.ii"
#include "task_manager.ii"
#include "vector.ii"


stop_task::stop_task(task_manager & n_owner):
  task(n_owner)
{
}


void stop_task::end()
{
  task_p_list::iterator i;

  for(i = m_tasks.begin(); i != m_tasks.end(); ++i)
  {
    (*i)->stop();
  }
}


