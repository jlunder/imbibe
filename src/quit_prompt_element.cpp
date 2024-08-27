#include "imbibe.h"

#include "quit_prompt_element.h"

#include "application.h"
#include "keyboard.h"


void quit_prompt_element::poll() {
}


bool quit_prompt_element::handle_key(uint16_t key) {
  switch (key) {
  case 'y':
  case 'Y':
  case 'q':
  case 'x':
  case key_code::enter:
    application::do_confirm_from_quit_prompt();
    return true;
  case key_code::escape:
    application::do_back_from_quit_prompt();
    return true;
  }
  return false;
}


void quit_prompt_element::animate(uint32_t delta_ms) {
  (void)delta_ms;
}


