#include "imbibe.h"

#include "key_manager.h"
#include "main_task.h"


#define logf_main_task(...) logf("MAIN_TASK: " __VA_ARGS__)


inplace<main_task> main_task_instance;


main_task::main_task()
  : task(), m_win(), m_main(), m_exit_requested(false) {
}


main_task::~main_task() {
}


void main_task::poll() {
  while(key_manager::key_event_available()) {
    uint16_t key = key_manager::read_key_event();
    logf_main_task("key pressed: %X\n", key);
    if (m_win.has_focus()) {
      element * e = &m_win.focus();
      while (!e->handle_key(key) && e->has_owner()
          && e->owner().is_element()) {
        e = &e->owner().as_element();
      }
    }
  }

  task_manager::request_run_task(*this);
}


void main_task::run() {
  // logf_main_task("run(), state=%d\n", m_state);

  m_win.lock_repaint();
  m_main.animate(m_frame_timer.reset_ms());
  // Do this after animate because coming into this method, the animated
  // positions may not have been set up yet
  if(!m_main.visible()) {
    m_main.show();
  }
  m_win.unlock_repaint();
}


void main_task::idle() {
}


void main_task::run_loop() {
  static uint32_t const min_poll_interval_ms = 10;
  static uint32_t const max_run_ms = 100;
  static uint32_t const max_idle_interval_ms = 200;
  timer idle_timer;
  timer poll_timer;

  timer::setup();
  m_win.setup();
  logf_main_task("imbibe 0.1 loaded\n");

  task_manager::idle();

  while (!m_exit_requested) {
#if defined(SIMULATE)
    step_simulator();
#endif

    uint32_t last_poll_ms = poll_timer.reset_ms();

    // idle() until it's time to run
    while (last_poll_ms + poll_timer.delta_ms() < min_poll_interval_ms) {
      task_manager::idle();
      idle_timer.reset_ms();
    }

    // run() until we're done or we've exhausted our budget
    bool any_tasks = task_manager::poll();
    while (any_tasks && (poll_timer.delta_ms() < max_run_ms)) {
      any_tasks = task_manager::run();
    }

    // idle() if it's been a really long time since we last did
    uint32_t ms_since_idle = idle_timer.delta_ms();
    // logf_main_task("ms_since_idle: %d\n", ms_since_idle);
    if (ms_since_idle >= max_idle_interval_ms) {
      task_manager::idle();
      idle_timer.reset_ms();
    }
  }

  logf_main_task("shutting down\n");
  m_win.teardown();
  // don't teardown key_manager b/c not needed
  timer::teardown();
  logf_main_task("bye!\n");
}


void main_task::exit() {
  logf_main_task("exit requested\n");
  main_task_instance->m_exit_requested = true;
}
