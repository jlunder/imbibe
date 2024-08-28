#include "imbibe.h"

#include "submenu_element.h"

#include "application.h"
#include "keyboard.h"

void submenu_element::layout(coord_t window_width, coord_t window_height) {
  set_frame(0, 0, window_width, window_height);
}

void submenu_element::poll() {}

bool submenu_element::handle_key(uint16_t key) {
  switch (key) {
  case key_code::escape:
    application::do_back_from_submenu();
    return true;
  }
  return false;
}

bool submenu_element::active() const { return false; }

void submenu_element::animate(anim_time_t delta_ms) { (void)delta_ms; }
