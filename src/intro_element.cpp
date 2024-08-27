#include "imbibe.h"

#include "intro_element.h"

#include "application.h"
#include "keyboard.h"


void intro_element::poll() {
}


bool intro_element::handle_key(uint16_t key) {
  switch (key) {
  case key_code::escape:
    application::do_next_from_intro();
    return true;
  }
  return false;
}


void intro_element::animate(uint32_t delta_ms) {
  (void)delta_ms;
  application::do_next_from_intro();
}


