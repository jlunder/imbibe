#include "imbibe.h"

#include "main_element.h"
#include "main_task.h"

#include "key_manager.h"


#define logf_main_element(...) logf("MAIN_ELEMENT: " __VA_ARGS__)


main_element::main_element()
  : window_element(), m_frame(), m_scroll() {
  add_element(m_frame);
  add_element(m_scroll);
}


main_element::~main_element() {
}


void main_element::animate(uint32_t delta_ms) {
  logf_main_element("animate: %lu ms\n", delta_ms);
  switch (m_state) {
  case st_intro:
    animate_intro(delta_ms);
    break;
  case st_main_menu:
    animate_main_menu(delta_ms);
    break;
  case st_outro:
    animate_outro(delta_ms);
    break;
  default:
    assert(!"invalid state");
    break;
  }

  // static uint32_t const seqs = 8;
  // static uint32_t const seq_ms = 4000;
  // static uint32_t const total_ms = seqs * seq_ms;

  // assert(delta_ms < total_ms);

  // m_anim_ms += delta_ms;
  // if(m_anim_ms >= total_ms) {
  //   if(m_anim_ms < total_ms * 2) {
  //     m_anim_ms -= total_ms;
  //   } else {
  //     m_anim_ms = 0;
  //   }
  // }

  // uint16_t seq = 0;
  // uint32_t seq_time = m_anim_ms;
  // static int16_t const t_shift = 8;
  // while(seq_time > seq_ms) {
  //   ++seq;
  //   seq_time -= seq_ms;
  // }
  // int16_t t = (uint16_t)((seq_time << t_shift) / seq_ms);
  // static int16_t const t_max = (1 << t_shift) - 1;
}


bool main_element::handle_key(uint16_t key) {
  if (key == key_event::escape) {
    main_task::exit();
    return true;
  }
  return false;
}


void main_element::animate_intro(uint32_t delta_ms) {
  static uint32_t const intro_ms = 500;
  static uint32_t const t_max = 1024;

  m_anim_ms += delta_ms;
  uint16_t anim_t = (uint16_t)min<uint32_t>(
    t_max, (m_anim_ms * t_max + intro_ms / 2) / intro_ms);

  if (anim_t >= t_max) {
    enter_main_menu();
  }
}


void main_element::animate_main_menu(uint32_t delta_ms) {
  (void)delta_ms;
  m_anim_ms = 0;
  enter_outro();
}


void main_element::animate_outro(uint32_t delta_ms) {
  static uint32_t const outro_ms = 500;
  static uint32_t const t_max = 1024;

  m_anim_ms += delta_ms;
  uint16_t anim_t = (uint16_t)min<uint32_t>(
    t_max, (m_anim_ms * t_max + outro_ms / 2) / outro_ms);

  if (anim_t >= t_max) {
    main_task::exit();
  }
}


void main_element::enter_main_menu() {
  logf_main_element("entering main_menu state\n");
  m_state = st_main_menu;
  m_anim_ms = 0;
}


void main_element::enter_outro() {
  logf_main_element("entering outro state\n");
  m_state = st_outro;
  m_anim_ms = 0;
}


