#include "imbibe.h"

#include "keyboard.h"
#include "main_task.h"
#include "resource_manager.h"
#include "timer.h"


#define logf_main_task(...) logf("MAIN_TASK: " __VA_ARGS__)


inplace<main_task> main_task_instance;


main_task::main_task()
  : task(), m_win(), m_main(), m_exit_requested(false) {
}


main_task::~main_task() {
}


void main_task::poll() {
  while (keyboard::key_event_available()) {
    uint16_t key = keyboard::read_key_event();
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
  //m_win.lock_repaint();
  m_main.animate(m_frame_timer.reset_ms());
  // Do this after animate because coming into this method, the animated
  // positions may not have been set up yet
  if (!m_main.visible()) {
    m_main.show();
  }
  //m_win.unlock_repaint();
  m_win.present();
}


void main_task::idle() {
}


void main_task::run_loop() {
  static uint32_t const min_poll_interval_ms = 10;
  static uint32_t const max_run_ms = 100;
  static uint32_t const max_idle_interval_ms = 200;
  timer idle_timer;
  timer poll_timer;

  bitmap * capture = new bitmap();
  m_win.setup(capture);
  m_main.set_captured_screen(im_ptr<bitmap>(capture));
  capture = NULL;
  m_main.set_frame(0, 0, m_win.backbuffer().width(), m_win.backbuffer().height());
  m_main.set_owner(m_win);
  m_main.layout();
  m_win.set_focus(m_main);

  logf_main_task("imbibe 0.1 loaded\n");

  task_manager::idle();

  while (!m_exit_requested) {
#if defined(SIMULATE)
    // logf_main_task("main loop: step_simulator\n");
    step_simulator();
#endif

    // logf_main_task("main loop: idle\n");
    // idle() until it's time to run
    while (!poll_timer.reset_if_elapsed(min_poll_interval_ms)) {
      task_manager::idle();
      idle_timer.reset_ms();
    }

    // logf_main_task("main loop: poll\n");
    // run() until we're done or we've exhausted our budget
    bool any_tasks = task_manager::poll();
    // logf_main_task("main loop: run\n");
    while (any_tasks && (poll_timer.delta_ms() < max_run_ms)) {
      any_tasks = task_manager::run();
    }

    // logf_main_task("main loop: check for starvation\n");
    // idle() if it's been a really long time since we last did
    uint32_t ms_since_idle = idle_timer.delta_ms();
    // logf_main_task("ms_since_idle: %d\n", ms_since_idle);
    if (ms_since_idle >= max_idle_interval_ms) {
      logf_main_task("main loop: starvation idle\n");
      task_manager::idle();
      idle_timer.reset_ms();
    }
  }

  logf_main_task("shutting down\n");
  m_win.teardown();
  logf_main_task("bye!\n");
}


void main_task::exit() {
  logf_main_task("exit requested\n");
  main_task_instance->m_exit_requested = true;
}
