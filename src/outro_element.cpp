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
  tbm goodbye_tbm(resource_manager::fetch_tbm(imstring("assets/goodbye.tbm")));
  m_goodbye.set_tbm(goodbye_tbm);
  m_goodbye.set_frame(0, 0, goodbye_tbm.width(), goodbye_tbm.height(), 2);
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
}

void outro_element::poll() {
  if (m_active && m_goodbye_y.done()) {
    application::do_next_from_outro();
    m_active = false;
  }
}

bool outro_element::handle_key(key_code_t key) {
  (void)key;
  if (m_active) {
    application::do_next_from_outro();
    m_active = false;
  }
  return false;
}

bool outro_element::active() const { return m_active; }

void outro_element::animate(anim_time_t delta_ms) {
  m_goodbye_y.update(delta_ms);
  coord_t goodbye_y = m_goodbye_y.value();
  m_goodbye.set_frame_pos(0, goodbye_y);
  m_logo.set_visible(m_goodbye.frame().y2 <= frame().height());
}

void outro_element::play_outro() {
  m_goodbye_y.reset(frame().height(), -m_goodbye.frame().height(), 10000);
  m_active = true;
}
