#include "cplusplus.hh"

#include "task.hh"

#include "task_manager.hh"

#include "task.ii"

#include "task_manager.ii"


task::task(task_manager & n_owner):
  m_owner(n_owner)
{
}


void task::begin()
{
}


void task::pre_cycle()
{
}


void task::cycle()
{
}


void task::post_cycle()
{
}


void task::end()
{
}


