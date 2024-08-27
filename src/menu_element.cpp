#include "imbibe.h"

#include "menu_element.h"


#include "application.h"
#include "keyboard.h"


void menu_element::poll() {
}


bool menu_element::handle_key(uint16_t key) {
  switch (key) {
  case key_code::escape:
    application::do_quit_from_anywhere();
    return true;
  }
  return false;
}


void menu_element::animate(uint32_t delta_ms) {
  (void)delta_ms;
}


