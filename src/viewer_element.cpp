#include "imbibe.h"

#include "viewer_element.h"

#include "application.h"
#include "keyboard.h"


void viewer_element::poll() {
}


bool viewer_element::handle_key(uint16_t key) {
  switch (key) {
  case key_code::escape:
    application::do_back_from_viewer();
    return true;
  }
  return false;
}


void viewer_element::animate(uint32_t delta_ms) {
  (void)delta_ms;
}


