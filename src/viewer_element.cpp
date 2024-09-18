#include "imbibe.h"

#include "viewer_element.h"

#include "application.h"
#include "keyboard.h"

void viewer_element::layout(coord_t window_width, coord_t window_height) {
  set_frame(0, 0, window_width, window_height);
}

void viewer_element::poll() {}

bool viewer_element::handle_key(key_code_t key) {
  switch (key) {
  case key_code::escape:
    application::do_back_from_viewer();
    return true;
  }
  return false;
}

bool viewer_element::active() const { return false; }

void viewer_element::animate(anim_time_t delta_ms) { (void)delta_ms; }
