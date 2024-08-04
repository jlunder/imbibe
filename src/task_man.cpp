#include "imbibe.h"

// #include "task_manager.h"
#include "task_man.h"

#include "task.h"
#include "vector.h"


delete_reclaim<action> action::s_default_reclaim;

do_nothing_reclaim<action> reusable_action::s_do_nothing_reclaim;

bool task_manager::s_busy = false;
task_manager::task_p_list task_manager::s_tasks;
task_manager::task_p_list task_manager::s_tasks_to_run;
task_manager::action_p_list task_manager::s_deferred_actions;
task_manager::task_p_list task_manager::s_tasks_to_reclaim;


void task_manager::add_task(task & t)
{
  assert(!s_busy);
  task_p_list::iterator e = s_tasks.end();
  for (task_p_list::iterator i = s_tasks.begin(); i != e; ++i) {
    if (*i == &t) {
      return;
    }
  }
  s_tasks.push_back(&t);
}


void task_manager::remove_task(task & t)
{
  assert(!s_busy);
  task_p_list::iterator e = s_tasks.end();
  for (task_p_list::iterator i = s_tasks.begin(); i != e; ) {
    if (*i != &t) {
      ++i;
    } else {
      --e;
      *i = *e;
    }
  }
  s_tasks.erase(e, s_tasks.end());
  t.reclaimer()(&t);
}


void task_manager::request_run_task(task & t)
{
  assert(!s_busy);
  task_p_list::iterator e = s_tasks.end();
  for (task_p_list::iterator i = s_tasks.begin(); i != e; ++i) {
    if (*i == &t) {
      return;
    }
  }
  s_tasks.push_back(&t);
}


void task_manager::defer_action(action * a)
{
  assert(s_busy);
  s_deferred_actions.push_back(a);
}


bool task_manager::run()
{
  assert(!s_busy);
  assert(s_deferred_actions.empty());

  if (s_tasks_to_run.empty()) {
    return false;
  }

  { // Run waiting tasks
    s_busy = true;
    task_p_list::iterator e = s_tasks_to_run.end();
    for (task_p_list::iterator i = s_tasks_to_run.begin(); i != e; ++i) {
      (*i)->run();
    }
    s_tasks_to_run.clear();
    s_busy = false;
  }

  { // Follow up with any deferred actions
    action_p_list::iterator e = s_deferred_actions.end();
    for (action_p_list::iterator i = s_deferred_actions.begin(); i != e; ++i) {
      (**i)();
      (*i)->reclaimer()(*i);
    }
    s_deferred_actions.clear();
  }

  return true;
}


void task_manager::idle()
{
  assert(!s_busy);
  assert(s_deferred_actions.empty());

  if (!s_tasks.empty()) {
    {
      s_busy = true;
      task_p_list::iterator e = s_tasks.end();
      for (task_p_list::iterator i = s_tasks.begin(); i != e; ++i) {
        (*i)->idle();
      }
      s_busy = false;
    }

    {
      action_p_list::iterator e = s_deferred_actions.end();
      for (action_p_list::iterator i = s_deferred_actions.begin(); i != e; ++i) {
        (**i)();
        (*i)->reclaimer()(*i);
      }
      s_deferred_actions.clear();
    }
  }
}


