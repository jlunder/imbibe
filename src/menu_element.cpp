#include "imbibe.h"

#include "menu_element.h"

#include "application.h"
#include "keyboard.h"
#include "resource_manager.h"
#include "tbm.h"
#include "termviz.h"

// first hotspot lines 11-17

menu_element::menu_element() {
  m_background.set_brush(termel::from('/', color::white, color::black));
}

menu_element::~menu_element() {}

void menu_element::layout(coord_t window_width, coord_t window_height) {
  set_frame(0, 0, window_width, window_height);
  unpacker header_tbm(resource_manager::fetch_tbm("menu-top.tbm"));
  tbm::dimensions(header_tbm, m_header_width, m_header_height);
  m_header.set_tbm(header_tbm);
  unpacker footer_tbm(resource_manager::fetch_tbm("menu-bot.tbm"));
  tbm::dimensions(footer_tbm, m_footer_width, m_footer_height);
  m_footer.set_tbm(footer_tbm);
  m_background.set_frame(0, 0, window_width, window_height, 0);
  m_header.set_frame(0, 0, window_width, m_header_height, 1);
  m_footer.set_frame(0, window_height - m_footer_height, window_width,
                     window_height, 2);
  m_header.set_owner(*this);
  m_header.show();
  m_footer.set_owner(*this);
  m_footer.show();
  m_background.set_owner(*this);
  m_background.show();
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
