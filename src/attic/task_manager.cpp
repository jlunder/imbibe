#include "imbibe.h"

#include "task_manager.h"

#include "task.h"
#include "vector.h"


#define logf_task_manager(...) disable_logf("TASK_MANAGER: " __VA_ARGS__)


namespace aux_task_manager {
  bool remove_task_from(vector<task *> & task_list, task * t) {
    vector<task *>::iterator e = task_list.end();
    for (vector<task *>::iterator i = task_list.begin(); i != e; ) {
      if (*i != t) {
        ++i;
      } else {
        logf_task_manager("found at %d\n", (int)(i - task_list.begin()));
        --e;
        *i = *e;
      }
    }
    if (e != task_list.end()) {
      logf_task_manager("erasing %d from end\n", (int)(task_list.end() - e));
      task_list.erase(e, task_list.end());
      task::reclaim_func_t rec = t->reclaimer();
      rec(t);
      return true;
    } else {
      return false;
    }
  }
}


namespace task_manager {
  typedef vector<task *> task_p_list;
  typedef vector<action *> action_p_list;

  bool s_busy = false;
  task_p_list s_tasks;
  task_p_list s_tasks_to_poll;
  task_p_list s_tasks_to_run;
  task_p_list s_tasks_to_idle;
  action_p_list s_deferred_actions;
}


void action::default_reclaim(action __far * p) {
  delete p;
}


void reusable_action::do_nothing_reclaim(action __far * p) {
  (void)p;
}


void task_manager::add_task(task & t)
{
  assert(!s_busy);
  logf_task_manager("adding task %p\n", &t);
  task_p_list::iterator e = s_tasks.end();
  for (task_p_list::iterator i = s_tasks.begin(); i != e; ++i) {
    if (*i == &t) {
      logf_task_manager("found already at %d\n", (int)(i - s_tasks.begin()));
      return;
    }
  }
  s_tasks.push_back(&t);
  s_tasks_to_poll.push_back(&t);
  s_tasks_to_idle.push_back(&t);
  logf_task_manager("added at %d: %p\n", (int)s_tasks.size() - 1,
    s_tasks.at(s_tasks.size() - 1));
}


void task_manager::remove_task(task & t)
{
  assert(!s_busy);
  logf_task_manager("removing task %p\n", &t);
  if (aux_task_manager::remove_task_from(s_tasks, &t)) {
    aux_task_manager::remove_task_from(s_tasks_to_poll, &t);
    aux_task_manager::remove_task_from(s_tasks_to_run, &t);
    aux_task_manager::remove_task_from(s_tasks_to_idle, &t);
  }
}


void task_manager::request_run_task(task & t)
{
  assert(!s_busy);
  logf_task_manager("request run task %p\n", &t);
  task_p_list::iterator e = s_tasks_to_run.end();
  for (task_p_list::iterator i = s_tasks_to_run.begin(); i != e; ++i) {
    if (*i == &t) {
      logf_task_manager("found already at %d\n", (int)(i - s_tasks.begin()));
      return;
    }
  }
  s_tasks_to_run.push_back(&t);
  logf_task_manager("added at %d: %p\n", (int)s_tasks_to_run.size() - 1,
    s_tasks_to_run.at(s_tasks_to_run.size() - 1));
}


void task_manager::defer_action(action * a)
{
  assert(s_busy);
  logf_task_manager("adding deferred action %p\n", a);
  s_deferred_actions.push_back(a);
}


bool task_manager::poll()
{
  assert(!s_busy);
  assert(s_deferred_actions.empty());
  // assert(s_tasks_to_run.empty());

  logf_task_manager("poll: polling %d tasks\n", (int)s_tasks_to_poll.size());
  {
    task_p_list::iterator e = s_tasks_to_poll.end();
    for (task_p_list::iterator i = s_tasks_to_poll.begin(); i != e; ++i) {
      logf_task_manager("polling task %p\n", *i);
      (*i)->poll();
    }
  }

  return !s_tasks_to_run.empty();
}


bool task_manager::run()
{
  assert(!s_busy);
  assert(s_deferred_actions.empty());

  if (s_tasks_to_run.empty()) {
    logf_task_manager("run: empty run list\n");
    return false;
  }

  logf_task_manager("run: executing %d tasks\n", (int)s_tasks_to_run.size());
  { // Run waiting tasks
    s_busy = true;
    task_p_list::iterator e = s_tasks_to_run.end();
    for (task_p_list::iterator i = s_tasks_to_run.begin(); i != e; ++i) {
      logf_task_manager("running task %p\n", *i);
      (*i)->run();
    }
    s_tasks_to_run.clear();
    s_busy = false;
  }

  { // Follow up with any deferred actions
    action_p_list::iterator e = s_deferred_actions.end();
    for (action_p_list::iterator i = s_deferred_actions.begin(); i != e;
        ++i) {
      logf_task_manager("running deferred action %p\n", *i);
      (**i)();
      action::reclaim_func_t rec = (*i)->reclaimer();
      rec(*i);
    }
    s_deferred_actions.clear();
  }

  return true;
}


void task_manager::idle()
{
  assert(!s_busy);
  assert(s_deferred_actions.empty());

  logf_task_manager("idle: idling tasks\n");
  if (!s_tasks_to_idle.empty()) {
    task_p_list::iterator e = s_tasks_to_idle.end();
    for (task_p_list::iterator i = s_tasks_to_idle.begin(); i != e; ++i) {
      logf_task_manager("idling task %p\n", *i);
      (*i)->idle();
    }
  }
}


