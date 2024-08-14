#include "imbibe.h"

#include "bitmap.h"
// #include "bitmap_graphics.h"
#include "bitmap_g.h"
// #include "key_manager.h"
#include "key_mana.h"
// #include "main_task.h"
#include "main_tas.h"


#define logf_main_task(...) disable_logf("MAIN_TASK: " __VA_ARGS__)


namespace aux_main_task {
  char const repaint_sequence[] = "-/|\\";
}


uint8_t main_task::repaint_element::m_counter = 0;


void main_task::repaint_element::paint(graphics & g) {
  g.draw_rectangle(0, 0, frame_width(), frame_height(),
    pixel(aux_main_task::repaint_sequence[m_counter], m_fill));
  m_counter = (uint8_t)(
    (m_counter + 1) % (LENGTHOF(aux_main_task::repaint_sequence) - 1));
}

void main_task::text_element::paint(graphics & g) {
  g.draw_rectangle(0, 0, frame_width(), frame_height(), m_fill);
  g.draw_text((frame_width() - (coord_t)strlen(m_message)) / 2,
    frame_height() / 2, m_text, m_message);
}


main_task::main_task()
  : task(), m_state(st_loading), m_win(), m_frame(),
    m_anim_time(0), m_background(color(color::hi_yellow, color::green)),
    m_clipper(), m_clip_background(color(color::hi_cyan, color::cyan)),
    m_orbit1(),
    m_orbit2(pixel('*', color(color::yellow, color::red)),
      color(color::hi_white, color::red), "Bjelo worlb?") {
  logf_main_task("m_frame = %p\n", &m_frame);
  logf_main_task("m_background = %p\n", &m_background);
  logf_main_task("m_clipper = %p\n", &m_clipper);
  logf_main_task("m_clip_background = %p\n", &m_clip_background);
  logf_main_task("m_orbit1 = %p\n", &m_orbit1);
  logf_main_task("m_orbit2 = %p\n", &m_orbit2);

  coord_t w = m_win.backbuffer().width();
  coord_t h = m_win.backbuffer().height();

  m_frame.set_frame(0, 0, w, h, 0);
  m_frame.set_owner(m_win);

  m_background.set_frame(0, 0, w, h, 0);
  m_background.set_owner(m_frame);
  m_background.show();

  m_clipper.set_frame(0, 0, w, h, 1);
  m_clipper.set_owner(m_frame);
  m_clip_background.set_frame(0, 0, w, h, 0);
  m_clip_background.set_owner(m_clipper);
  m_clip_background.show();
  // coord_t ow = 16;
  // coord_t oh = 5;
  // {
  //   m_orbit1.set_frame(0, 0, ow, oh, 1);
  //   m_orbit1.set_b(new bitmap(ow, oh));
  //   bitmap_graphics g(m_orbit1.b());
  //   g.draw_rectangle(0, 0, ow, oh,
  //     pixel('+', color(color::cyan, color::blue)));
  //   g.draw_text(1, 2, color(color::hi_white, color::blue), "Hello world!");
  //   m_orbit1.set_owner(m_clipper);
  //   m_orbit1.show();
  // }
  // m_orbit2.set_frame(0, 0, ow, oh, 2);
  // m_orbit2.set_owner(m_clipper);
  // m_orbit2.show();

  m_clipper.show();

  // Don't show m_frame until animate()
}


main_task::~main_task() {
}


void main_task::poll() {
  key_manager::dispatch_keys();
  task_manager::request_run_task(*this);
}


void main_task::run() {
  // logf_main_task("run(), state=%d\n", m_state);

  m_win.lock_repaint();
  animate(m_frame_timer.reset_ms());
  m_win.unlock_repaint();

  switch (m_state) {
  case st_loading:
    logf_main_task("advancing to st_waiting\n");
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
}


bool main_task::handle_key(uint16_t key) {
  (void)key;
  logf_main_task("pressed [this=%p, m_state=%d]: %x\n", this, (int)m_state,
    key);
  if (m_state == st_waiting) {
    m_state = st_done;
  }
  return true;
}


void main_task::animate(uint32_t delta_ms) {
  static int16_t const seqs = 8;
  static int16_t const seq_ms = 1000;
  static int16_t const total_ms = seqs * seq_ms;

  assert(delta_ms < total_ms);
  if(delta_ms > total_ms) {
    delta_ms = 0;
  }

  m_anim_time += (int16_t)delta_ms;
  while(m_anim_time >= total_ms) {
    m_anim_time -= total_ms;
  }

  int16_t seq = 0;
  int16_t t = m_anim_time;
  static int16_t const t_shift = 12;
  while(t > seq_ms) {
    ++seq;
    t -= seq_ms;
  }
  t = (int16_t)(((int32_t)t << t_shift) / seq_ms);

  coord_t w = m_frame.frame_width();
  coord_t h = m_frame.frame_height();

  if (seq < (seqs / 2)) {
    m_clipper.set_frame(0, 0, w, h, 1);
  } else {
    m_clipper.set_frame(w * 1 / 4, h * 1 / 4, w * 3 / 4, h * 3 / 4, 1);
  }

  // coord_t ow = m_orbit1.frame_width();
  // coord_t oh = m_orbit1.frame_height();
  // static int16_t const t_max = (1 << t_shift) - 1;

  // switch(seq % (seqs / 2)) {
  // case 0:
  //   m_orbit1.set_frame_pos(
  //     (coord_t)(((uint16_t)(w + ow) * t) >> t_shift) - ow,
  //     (h * 2 / 4) - oh / 2);
  //   m_orbit2.set_frame_pos(
  //     (coord_t)(((uint16_t)(w + ow) * (t_max - t)) >> t_shift) - ow,
  //     (h * 2 / 4) - (oh + 1) / 2);
  //   break;
  // case 1:
  //   m_clipper.lock_repaint();
  //   m_orbit1.set_frame_pos(
  //     (coord_t)(((uint16_t)(w + ow) * t) >> t_shift) - ow,
  //     (h * 1 / 4) - oh / 2);
  //   m_orbit2.set_frame_pos(
  //     (coord_t)(((uint16_t)(w + ow) * (t_max - t)) >> t_shift) - ow,
  //     (h * 3 / 4) - (oh + 1) / 2);
  //   m_clipper.unlock_repaint();
  //   break;
  // case 2:
  //   m_orbit1.set_frame_pos(
  //     (w * 2 / 4) - ow / 2,
  //     (coord_t)(((uint16_t)(h + oh) * t) >> t_shift) - oh);
  //   m_orbit2.set_frame_pos(
  //     (w * 2 / 4) - (ow + 1) / 2,
  //     (coord_t)(((uint16_t)(h + oh) * (t_max - t)) >> t_shift) - oh);
  //   break;
  // case 3:
  //   m_clipper.lock_repaint();
  //   m_orbit1.set_frame_pos(
  //     (w * 1 / 4) - ow / 2,
  //     (coord_t)(((uint16_t)(h + oh) * t) >> t_shift) - oh);
  //   m_orbit2.set_frame_pos(
  //     (w * 3 / 4) - (ow + 1) / 2,
  //     (coord_t)(((uint16_t)(h + oh) * (t_max - t)) >> t_shift) - oh);
  //   m_clipper.unlock_repaint();
  //   break;
  // }

  // Do this at the end because coming into this method, the animated
  // positions may not have been set up yet
  if(!m_frame.visible()) {
    m_frame.show();
  }
}


void main_task::run_loop() {
  static uint32_t const min_poll_interval_ms = 1000;
  static uint32_t const max_run_ms = 100;
  static uint32_t const max_idle_interval_ms = 200;
  timer idle_timer;
  timer poll_timer;

  timer::setup();
  key_manager::add_handler(*this);
  m_win.setup();
  logf_main_task("imbibe 0.1 loaded\n");

  task_manager::idle();
  logf_main_task("starting run [this=%p, m_state=%d]\n", this, (int)m_state);

  while (!done()) {
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
