#include "imbibe.h"

// #include "task_manager.h"
#include "task_man.h"

#include "task.h"
#include "vector.h"


// #define logf cprintf


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
  logf("adding task %p\n", &t);
  task_p_list::iterator e = s_tasks.end();
  for (task_p_list::iterator i = s_tasks.begin(); i != e; ++i) {
    if (*i == &t) {
      logf("found already at %d\n", (int)(i - s_tasks.begin()));
      return;
    }
  }
  s_tasks.push_back(&t);
  logf("added at %d: %p\n", (int)s_tasks.size() - 1, s_tasks.at(s_tasks.size() - 1));
}


void task_manager::remove_task(task & t)
{
  assert(!s_busy);
  logf("removing task %p\n", &t);
  task_p_list::iterator e = s_tasks.end();
  for (task_p_list::iterator i = s_tasks.begin(); i != e; ) {
    if (*i != &t) {
      ++i;
    } else {
      logf("found at %d\n", (int)(i - s_tasks.begin()));
      --e;
      *i = *e;
    }
  }
  if (e != s_tasks.end()) {
    logf("erasing %d from end\n", (int)(s_tasks.end() - e));
    s_tasks.erase(e, s_tasks.end());
    t.reclaimer()(&t);
  }
}


void task_manager::request_run_task(task & t)
{
  assert(!s_busy);
  logf("request run task %p\n", &t);
  task_p_list::iterator e = s_tasks_to_run.end();
  for (task_p_list::iterator i = s_tasks_to_run.begin(); i != e; ++i) {
    if (*i == &t) {
      logf("found already at %d\n", (int)(i - s_tasks.begin()));
      return;
    }
  }
  s_tasks_to_run.push_back(&t);
  logf("added at %d: %p\n", (int)s_tasks_to_run.size() - 1,
    s_tasks_to_run.at(s_tasks_to_run.size() - 1));
}


void task_manager::defer_action(action * a)
{
  assert(s_busy);
  logf("adding deferred action %p\n", a);
  s_deferred_actions.push_back(a);
}


bool task_manager::run()
{
  assert(!s_busy);
  assert(s_deferred_actions.empty());

  if (s_tasks_to_run.empty()) {
    logf("run: empty run list\n");
    return false;
  }

  logf("run: executing %d tasks\n", (int)s_tasks_to_run.size());
  { // Run waiting tasks
    s_busy = true;
    task_p_list::iterator e = s_tasks_to_run.end();
    for (task_p_list::iterator i = s_tasks_to_run.begin(); i != e; ++i) {
      logf("running task %p\n", *i);
      (*i)->run();
    }
    s_tasks_to_run.clear();
    s_busy = false;
  }

  { // Follow up with any deferred actions
    action_p_list::iterator e = s_deferred_actions.end();
    for (action_p_list::iterator i = s_deferred_actions.begin(); i != e; ++i) {
      logf("running deferred action %p\n", *i);
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

  logf("idle: idling tasks\n");
  if (!s_tasks.empty()) {
    {
      s_busy = true;
      task_p_list::iterator e = s_tasks.end();
      for (task_p_list::iterator i = s_tasks.begin(); i != e; ++i) {
        logf("idling task %p\n", *i);
        (*i)->idle();
      }
      s_busy = false;
    }

    {
      action_p_list::iterator e = s_deferred_actions.end();
      for (action_p_list::iterator i = s_deferred_actions.begin(); i != e; ++i) {
        logf("idling deferred action %p\n", *i);
        (**i)();
        (*i)->reclaimer()(*i);
      }
      s_deferred_actions.clear();
    }
  }
}


