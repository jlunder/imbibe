#include "imbibe.h"

#include "bitmap.h"
#include "graphics.h"
#include "keyboard.h"
#include "render_test_task.h"


#define logf_render_test_task(...) logf("RENDER_TEST_TASK: " __VA_ARGS__)


namespace aux_main_task {
  // char const repaint_sequence[] = "-\\|/";
  char const repaint_sequence[] = "0123456789ABCDEFG";
}


inplace<render_test_task> render_task_instance;

uint8_t render_test_task::repaint_element::m_counter = 0;


void render_test_task::repaint_element::paint(graphics & g) {
  g.draw_rectangle(0, 0, frame_width(), frame_height(),
    termel::from(aux_main_task::repaint_sequence[m_counter], m_fill));
  ++m_counter;
  if (m_counter >= LENGTHOF(aux_main_task::repaint_sequence) - 1) {
    m_counter = 0;
  }
}


void render_test_task::text_element::paint(graphics & g) {
  logf_render_test_task("paint text_element at %d, %d; clip %d, %d, %d, %d; "
      "frame %d, %d, %d, %d\n",
    g.origin_x(), g.origin_y(), g.clip_x1(), g.clip_y1(), g.clip_x2(),
    g.clip_y2(), frame_x1(), frame_y1(), frame_x2(), frame_y2());
  g.draw_rectangle(0, 0, frame_width(), frame_height(), m_fill);
  g.draw_text((frame_width() - (coord_t)strlen(m_message)) / 2,
    frame_height() / 2, m_text, m_message);
}


render_test_task::render_test_task()
  : task(), m_state(st_loading), m_win(), m_frame(),
    m_anim_time(0),
    m_background(attribute::from(color::hi_yellow, color::green)),
    m_clipper(),
    m_clip_background(attribute::from(color::hi_cyan, color::cyan)),
    m_orbit1(),
    m_orbit2(termel::from('*', color::yellow, color::red),
      attribute::from(color::hi_white, color::red), "Bjelo worlb?") {
  logf_render_test_task("m_frame = %p\n", &m_frame);
  logf_render_test_task("m_background = %p\n", &m_background);
  logf_render_test_task("m_clipper = %p\n", &m_clipper);
  logf_render_test_task("m_clip_background = %p\n", &m_clip_background);
  logf_render_test_task("m_orbit1 = %p\n", &m_orbit1);
  logf_render_test_task("m_orbit2 = %p\n", &m_orbit2);

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
  coord_t ow = 16;
  coord_t oh = 5;
  {
    m_orbit1.set_frame(0, 0, ow, oh, 1);
    bitmap * b = new bitmap(ow, oh);
    graphics g(*b);
    g.draw_rectangle(0, 0, ow, oh,
      termel::from('+', color::cyan, color::blue));
    g.draw_text(2, 2, attribute::from(color::hi_white, color::blue),
      "Hello world!");
    m_orbit1.set_b(im_ptr<bitmap>(b));
    b = NULL;
    m_orbit1.set_owner(m_clipper);
    m_orbit1.show();
  }
  m_orbit2.set_frame(0, 0, ow, oh, 2);
  m_orbit2.set_owner(m_clipper);
  m_orbit2.show();

  m_clipper.show();

  // Don't show m_frame until animate()
}


render_test_task::~render_test_task() {
}


void render_test_task::poll() {
  while (keyboard::key_event_available()) {
    if (keyboard::read_key_event() == key_code::escape) {
      m_state = st_done;
    }
  }
  task_manager::request_run_task(*this);
}


void render_test_task::run() {
  // logf_render_test_task("run(), state=%d\n", m_state);

  m_win.lock_repaint();
  animate(m_frame_timer.reset_ms());
  m_win.unlock_repaint();

  switch (m_state) {
  case st_loading:
    logf_render_test_task("advancing to st_waiting\n");
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


void render_test_task::idle() {
}


void render_test_task::animate(uint32_t delta_ms) {
  static uint32_t const seqs = 8;
  static uint32_t const seq_ms = 4000;
  static uint32_t const total_ms = seqs * seq_ms;

  assert(delta_ms < total_ms);

  m_anim_time += delta_ms;
  if(m_anim_time >= total_ms) {
    if(m_anim_time < total_ms * 2) {
      m_anim_time -= total_ms;
    } else {
      m_anim_time = 0;
    }
  }

  uint16_t seq = 0;
  uint32_t seq_time = m_anim_time;
  static int16_t const t_shift = 8;
  while(seq_time > seq_ms) {
    ++seq;
    seq_time -= seq_ms;
  }
  int16_t t = (uint16_t)((seq_time << t_shift) / seq_ms);
  static int16_t const t_max = (1 << t_shift) - 1;

  coord_t w = m_frame.frame_width();
  coord_t h = m_frame.frame_height();
  coord_t ow = m_orbit1.frame_width();
  coord_t oh = m_orbit1.frame_height();

  coord_t anim_x = (((w + ow) * t) >> t_shift) - ow;
  coord_t anim_y = (((h + oh) * t) >> t_shift) - oh;
  coord_t anim_rx = (((w + ow) * (t_max - t)) >> t_shift) - ow;
  coord_t anim_ry = (((h + oh) * (t_max - t)) >> t_shift) - oh;

  if (seq < (seqs / 2)) {
    m_clipper.set_frame(0, 0, w, h, 1);
    m_clipper.set_offset_pos(0, 0);
  } else {
    m_clipper.set_frame(w * 1 / 4, h * 1 / 4, w * 3 / 4, h * 3 / 4, 1);
    m_clipper.set_offset_pos(-(w * 1 / 4), -(h * 1 / 4));
  }

  switch(seq % (seqs / 2)) {
  case 0:
    m_orbit1.set_frame_pos(anim_x, (h * 2 / 4) - (oh + 1) / 2);
    m_orbit2.set_frame_pos(anim_rx, (h * 2 / 4) - (oh + 1) / 2);
    break;
  case 1:
    m_clipper.lock_repaint();
    m_orbit1.set_frame_pos(anim_x, (h * 1 / 4) - (oh + 1) / 2);
    m_orbit2.set_frame_pos(anim_rx, (h * 3 / 4) - (oh + 1) / 2);
    m_clipper.unlock_repaint();
    break;
  case 2:
    m_orbit1.set_frame_pos((w * 2 / 4) - (ow + 1) / 2, anim_y);
    m_orbit2.set_frame_pos((w * 2 / 4) - (ow + 1) / 2, anim_ry);
    break;
  case 3:
    m_clipper.lock_repaint();
    m_orbit1.set_frame_pos((w * 1 / 4) - (ow + 1) / 2, anim_y);
    m_orbit2.set_frame_pos((w * 3 / 4) - (ow + 1) / 2, anim_ry);
    m_clipper.unlock_repaint();
    break;
  }

  // Do this at the end because coming into this method, the animated
  // positions may not have been set up yet
  if(!m_frame.visible()) {
    m_frame.show();
  }
}


void render_test_task::run_loop() {
  static uint32_t const min_poll_interval_ms = 10;
  static uint32_t const max_run_ms = 100;
  static uint32_t const max_idle_interval_ms = 200;
  timer idle_timer;
  timer poll_timer;

  timer::setup();
  m_win.setup();
  logf_render_test_task("imbibe 0.1 loaded\n");

  task_manager::idle();
  logf_render_test_task("starting run [this=%p, m_state=%d]\n", this, (int)m_state);

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
    // logf_render_test_task("ms_since_idle: %d\n", ms_since_idle);
    if (ms_since_idle >= max_idle_interval_ms) {
      task_manager::idle();
      idle_timer.reset_ms();
    }
  }

  logf_render_test_task("shutting down\n");
  m_win.teardown();
  // don't teardown keyboard b/c not needed
  timer::teardown();
  logf_render_test_task("bye!\n");
}
