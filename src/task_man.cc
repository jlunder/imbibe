#include "cplusplus.hh"

#include "task_man.hh"

#include "task.hh"
#include "vector.hh"

#include "task_man.ii"

#include "task.ii"
#include "vector.ii"


task_manager::task_manager():
  m_busy(false)
{
}


void task_manager::add_task(task & t)
{
  if(!m_busy)
  {
    m_tasks.push_back(&t);
    t.begin();
  }
  else
  {
    m_tasks_to_add.push_back(&t);
  }
}


void task_manager::remove_task(task & t)
{
  task_p_list::iterator i;

  if(!m_busy)
  {
    t.end();
    for(i = m_tasks.begin(); (i != m_tasks.end()) && (*i != &t); ++i);
    assert(i != m_tasks.end());
    m_tasks.erase(i);
  }
  else
  {
    m_tasks_to_remove.push_back(&t);
  }
}


void task_manager::run()
{
  task_p_list::iterator i;
  unsigned long l = 0;

  while(!m_tasks.empty())
  {
    l = 0;
    m_busy = true;
    for(i = m_tasks.begin(); i != m_tasks.end(); ++i)
    {
      (*i)->pre_cycle();
    }
    for(i = m_tasks.begin(); i != m_tasks.end(); ++i)
    {
      (*i)->cycle();
    }
    for(i = m_tasks.begin(); i != m_tasks.end(); ++i)
    {
      (*i)->post_cycle();
    }
    m_busy = false;
    for(i = m_tasks_to_add.begin(); i != m_tasks_to_add.end(); ++i)
    {
      add_task(**i);
    }
    m_tasks_to_add.clear();
    for(i = m_tasks_to_remove.begin(); i != m_tasks_to_remove.end(); ++i)
    {
      remove_task(**i);
    }
    m_tasks_to_remove.clear();
  }
}


