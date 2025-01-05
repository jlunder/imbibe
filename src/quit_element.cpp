#include "imbibe.h"

#include "quit_element.h"

#include "application.h"
#include "keyboard.h"
#include "resource_manager.h"
#include "tbm.h"
#include "unpacker.h"

quit_element::quit_element() {
  m_quit_tbm = resource_manager::fetch_tbm(imstring("assets/quit.tbm"));
  m_active = false;
}

void quit_element::layout(coord_t window_width, coord_t window_height) {
  set_frame(0, 0, window_width, window_height);
}

void quit_element::poll() {}

bool quit_element::handle_key(key_code_t key) {
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

bool quit_element::active() const { return m_active; }

void quit_element::animate(anim_time_t delta_ms) { (void)delta_ms; }

void quit_element::prompt_quit() { m_active = true; }

void quit_element::paint(graphics *g) {
  g->blend_rectangle(frame(), termel::from(' ', color::black, color::black), 5);
  coord_t quit_x = (frame().width() - m_quit_tbm.width()) / 2;
  coord_t quit_y = (frame().height() - m_quit_tbm.height()) / 2;
  g->draw_tbm(quit_x, quit_y, m_quit_tbm);
}