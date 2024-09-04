#include "imbibe.h"

#include "quit_prompt_element.h"

#include "application.h"
#include "keyboard.h"
#include "resource_manager.h"
#include "tbm.h"
#include "unpacker.h"

quit_prompt_element::quit_prompt_element() {
  tbm quit_tbm(resource_manager::fetch_tbm("quit.tbm"));
  tbm_header const __far &h = quit_tbm.header();
  m_quit_width = h.width;
  m_quit_height = h.height;
  m_quit.set_tbm(quit_tbm);
  m_active = false;
}

void quit_prompt_element::layout(coord_t window_width, coord_t window_height) {
  set_frame(0, 0, window_width, window_height);
  coord_t quit_x = (window_width - m_quit_width) / 2;
  coord_t quit_y = (window_height - m_quit_height) / 2;
  m_quit.set_frame(quit_x, quit_y, quit_x + m_quit_width,
                   quit_y + m_quit_height);
  m_quit.set_owner(this);
  m_quit.show();
}

void quit_prompt_element::poll() {}

bool quit_prompt_element::handle_key(uint16_t key) {
  switch (key) {
  case 'y':
  case 'Y':
  case 'q':
  case 'x':
  case key_code::enter:
    application::do_confirm_from_quit_prompt();
    m_active = false;
    return true;
  case 'n':
  case 'N':
  case key_code::escape:
    application::do_back_from_quit_prompt();
    m_active = false;
    return true;
  }
  return false;
}

bool quit_prompt_element::active() const { return m_active; }

void quit_prompt_element::animate(anim_time_t delta_ms) { (void)delta_ms; }

void quit_prompt_element::prompt_quit() { m_active = true; }
