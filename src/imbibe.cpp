#include "imbibe.h"

#include <stdlib.h>

// #include "key_manager.h"
#include "key_mana.h"
#include "task.h"
// #include "task_manager.h"
#include "task_man.h"
// #include "text_window.h"
#include "text_win.h"
#include "timer.h"


#define logf cprintf


class main_task: public task, public key_handler {
public:
  main_task();
  bool done() { return m_state == st_done; }
  virtual void run();
  virtual void idle();
  virtual bool handle_key(uint16_t key);

private:
  enum state_t {
    st_loading,
    st_waiting,
    st_done
  };

  state_t m_state;
};


main_task::main_task(): task(), m_state(st_loading) {
}


void main_task::run() {
  logf("run(), state=%d\n", m_state);
  switch(m_state) {
  case st_loading:
    logf("advancing to st_waiting\n");
    m_state = st_waiting;
    break;
  case st_waiting:
    break;
  case st_done:
    break;
  default:
    assert(!"invalid state");
    break;
  }
}


void main_task::idle() {
  key_manager::dispatch_keys();
}


bool main_task::handle_key(uint16_t key) {
  logf("pressed: %x\n", key);
  if(m_state == st_waiting) {
    m_state = st_done;
  }
  return true;
}


main_task main_instance;


void run() {
  static uint32_t const max_idle_interval = 50;
  timer t;
  uint32_t ms_since_idle = 0;

  task_manager::idle();
  while(!main_instance.done()) {
    // This loop is very carefully arranged. We attempt run at least every
    // cycle, and if nothing is available, we also idle. If things are
    // running constantly, we try to idle at least once every
    // max_idle_interval.
    bool do_idle = !task_manager::run();
    // This check is after run() to capture the most recent view of how much
    // time has been consumed. run() returns false when it did no work.
    ms_since_idle += t.delta_ms();
    if(ms_since_idle > max_idle_interval) {
      do_idle = true;
    }
    if(do_idle) {
      task_manager::idle();
      // The lower bound of max_idle_interval should be obvious. The upper
      // bound of *2 is to prevent the idle from accruing a backlog that
      // takes arbitrarily long to clear, if run is chronically exceeding its
      // time budget.
      if(ms_since_idle > max_idle_interval
          && ms_since_idle < max_idle_interval * 2) {
        // We don't just reset the ms_since_idle because we want to maintain
        // a particular frequency of idle polls overall regardless of how
        // much time the idle itself takes. The other way is legit, but then
        // the promise is that the start of the next idle will be around
        // max_idle_interval from the *end* of this idle, not the *start*.
        ms_since_idle -= max_idle_interval;
      } else {
        ms_since_idle = 0;
      }
    }
  }
}

int main(int argc, char * argv[]) {
  timer::setup();
  //m.start(true);
  key_manager::add_handler(main_instance);
  main_instance.start();
  logf("imbibe 1.0 loaded\n");
  run();
  logf("imbibe 1.0 done\n");
  timer::teardown();
  return 0;
}


