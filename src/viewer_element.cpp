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

void viewer_element::activate(imstring const &view_label,
                              imstring const &view_path) {
  if (m_transition_in_out.done()) {
    assert(!"should be running a transition animation by now");
    m_transition_in_out.reset(0);
  }

  m_view_path = view_path;
  m_view_label = view_label;

  m_active = true;
}

void viewer_element::deactivate() { m_active = false; }

void viewer_element::enter_from_menu() {
  m_transition_in_out.reset(frame().width(), 0, 300);
  m_scroll_y.reset(0);
}

void viewer_element::leave_to_menu() {
  m_transition_in_out.reset(0, frame().width(), 300);
}
