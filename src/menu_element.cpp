#include "imbibe.h"

#include "menu_element.h"

#include "application.h"
#include "keyboard.h"

void menu_element::layout(coord_t window_width, coord_t window_height) {
  set_frame(0, 0, window_width, window_height);
}

void menu_element::poll() {}

bool menu_element::handle_key(uint16_t key) {
  switch (key) {
  case key_code::escape:
    application::do_quit_from_anywhere();
    return true;
  }
  return false;
}

bool menu_element::active() const { return true; }

void menu_element::animate(anim_time_t delta_ms) { (void)delta_ms; }
