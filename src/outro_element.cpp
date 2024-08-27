#include "imbibe.h"

#include "outro_element.h"

#include "keyboard.h"
#include "application.h"


void outro_element::poll() {
}


bool outro_element::handle_key(uint16_t key) {
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


void outro_element::animate(uint32_t delta_ms) {
  (void)delta_ms;
  application::do_next_from_outro();
}


