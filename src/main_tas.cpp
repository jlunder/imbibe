#include "imbibe.h"

#include "bitmap.h"
// #include "bitmap_graphics.h"
#include "bitmap_g.h"
// #include "key_manager.h"
#include "key_mana.h"
// #include "main_task.h"
#include "main_tas.h"
#include "timer.h"


#undef logf
#define logf cprintf


main_task::main_task(): task(), m_state(st_loading), m_win(), m_canvas() {
  m_canvas.set_frame(0, 0, 80, 25, 0);
  m_canvas.set_b(new bitmap(80, 25));
  // bitmap_graphics g(m_canvas.b());
  // g.draw_rectangle(0, 0, 80, 25, pixel('+', color(color::hi_red, color::red)));
  // g.draw_text(34, 12, color(color::hi_white, color::white), "Hello world!");

  for(size_t i = 0; i < 80 * 25; ++i) {
    m_canvas.b().data()[i] = pixel('x', color(color::hi_green, color::red));
  }

  // m_canvas.set_owner(m_win);
  // m_canvas.show();
}


main_task::~main_task() {
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

#include <string.h>
void main_task::idle() {
  key_manager::dispatch_keys();

  //m_win.repaint();

  uint16_t * scr = (uint16_t *)MK_FP(0xB800, (void *)0);
  // for(size_t i = 0; i < 80 * 25; ++i) {
  //   scr[i] = pixel('x', color(color::cyan, color::magenta));
  // }
  for(size_t i = 0; i < 80 * 25; ++i) {
    m_canvas.b().data()[i] = pixel('x', color(color::cyan, color::red));
  }
  bitmap_graphics g(m_canvas.b());
  g.draw_rectangle(0, 0, 80, 25, pixel('+', color(color::hi_red, color::red)));
  //g.draw_text(34, 12, color(color::hi_white, color::white), "Hello world!");
  memcpy(scr, m_canvas.b().data(), 8000);
}


bool main_task::handle_key(uint16_t key) {
  logf("pressed: %x\n", key);
  if(m_state == st_waiting) {
    m_state = st_done;
  }
  return true;
}


void main_task::run_loop() {
  static uint32_t const max_idle_interval = 50;
  timer t;
  uint32_t ms_since_idle = 0;

  timer::setup();
  //m.start(true);
  logf("w=%d, h=%d\n", (int)m_canvas.b().width(), (int)m_canvas.b().height());
  key_manager::add_handler(*this);
  logf("w=%d, h=%d\n", (int)m_canvas.b().width(), (int)m_canvas.b().height());
  m_win.setup();
  logf("imbibe 1.0 loaded\n");

  task_manager::idle();
  while(!done()) {
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

  logf("imbibe 1.0 done\n");
  m_win.teardown();
  // don't teardown key_manager b/c not needed
  timer::teardown();
}
