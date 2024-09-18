#include "imbibe.h"

#include "outro_element.h"

#include "application.h"
#include "keyboard.h"

void outro_element::layout(coord_t window_width, coord_t window_height) {
  set_frame(0, 0, window_width, window_height);
  m_active = false;
}

void outro_element::poll() { application::do_next_from_outro(); }

bool outro_element::handle_key(key_code_t key) {
  switch (key) {
  case key_code::escape:
  case 'q':
  case 'x':
  case 'Q': // shift_q
  case 'X': // shift_x
  case key_code::control_q:
  case key_code::control_x:
  case key_code::alt_q:
  case key_code::alt_x:
    application::do_next_from_outro();
    return true;
  }
  return false;
}

bool outro_element::active() const { return m_active; }

void outro_element::animate(anim_time_t delta_ms) { (void)delta_ms; }

void outro_element::play_outro() { m_active = true; }
