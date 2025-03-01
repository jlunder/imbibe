#include "imbibe.h"

#include "outro_element.h"

#include "application.h"
#include "keyboard.h"
#include "resource_manager.h"
#include "tbm.h"

outro_element::outro_element() {
  tbm logo_tbm(resource_manager::fetch_tbm(imstring("assets/postlogo.tbm")));
  m_logo.set_tbm(logo_tbm);
  m_logo.set_frame(0, 0, logo_tbm.width(), logo_tbm.height(), 1);
  tbm goodbye_tbm(resource_manager::fetch_tbm(imstring("6gallery/viewer/CODEFENIX-WASH_THE_DISHES.tbm")));
  m_goodbye.set_tbm(goodbye_tbm);
  m_goodbye.set_frame(0, 0, goodbye_tbm.width(), goodbye_tbm.height(), 2);
  m_background.set_brush(termel::from(' ', color::white, color::black));
  m_active = false;
}

void outro_element::layout(coord_t window_width, coord_t window_height) {
  set_frame(0, 0, window_width, window_height);
  m_logo.set_frame_pos((window_width - m_logo.frame().width()) / 2,
                       (window_height - m_logo.frame().height()) / 2);
  m_logo.set_owner(this);
  m_logo.show();
  m_goodbye.set_owner(this);
  m_goodbye.show();
  m_background.set_frame(0, 0, window_width, window_height, 0);
  m_background.set_owner(this);
}

void outro_element::poll() {
  if (m_active && m_goodbye_y.done()) {
    application::do_next_from_outro();
  }
}

bool outro_element::handle_key(key_code_t key) {
  (void)key;
  if (m_active) {
    application::do_next_from_outro();
  }
  return false;
}

bool outro_element::active() const { return m_active; }

bool outro_element::opaque() const { return m_goodbye.frame().y1 <= 0; }

void outro_element::animate(anim_time_t delta_ms) {
  m_goodbye_y.update(delta_ms);
  coord_t goodbye_y = m_goodbye_y.value();
  m_goodbye.set_frame_pos(0, goodbye_y);
  bool background_visible = m_goodbye.frame().y2 <= frame().height();
  m_logo.set_visible(background_visible);
  m_background.set_visible(background_visible);
}

void outro_element::play_outro() {
  m_goodbye_y.reset(frame().height(), -m_goodbye.frame().height(),
                    120 * m_goodbye.frame().height());
  m_active = true;
}

void outro_element::finish_outro() { m_goodbye_y.finish(); }
