#include "imbibe.h"

#include "submenu_element.h"

#include "application.h"
#include "keyboard.h"


void submenu_element::poll() {
}


bool submenu_element::handle_key(uint16_t key) {
  switch (key) {
  case key_code::escape:
    application::do_back_from_submenu();
    return true;
  }
  return false;
}


void submenu_element::animate(uint32_t delta_ms) {
  (void)delta_ms;
}



