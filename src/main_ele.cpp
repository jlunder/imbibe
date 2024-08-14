#include "imbibe.h"

// #include "main_element.h"
#include "main_ele.h"


main_element::main_element()
  : window_element(), m_frame(), m_scroll() {
  add_element(m_frame);
  add_element(m_scroll);
}


main_element::~main_element() {
}


void main_element::animate(uint32_t delta_ms) {
  (void)delta_ms;
  // static uint32_t const seqs = 8;
  // static uint32_t const seq_ms = 4000;
  // static uint32_t const total_ms = seqs * seq_ms;

  // assert(delta_ms < total_ms);

  // m_anim_time += delta_ms;
  // if(m_anim_time >= total_ms) {
  //   if(m_anim_time < total_ms * 2) {
  //     m_anim_time -= total_ms;
  //   } else {
  //     m_anim_time = 0;
  //   }
  // }

  // uint16_t seq = 0;
  // uint32_t seq_time = m_anim_time;
  // static int16_t const t_shift = 8;
  // while(seq_time > seq_ms) {
  //   ++seq;
  //   seq_time -= seq_ms;
  // }
  // int16_t t = (uint16_t)((seq_time << t_shift) / seq_ms);
  // static int16_t const t_max = (1 << t_shift) - 1;
}
