#include "imbibe.h"

#include "intro_element.h"

#include "application.h"
#include "keyboard.h"
#include "resource_manager.h"
#include "tbm.h"

intro_element::intro_element() {
  tbm logo_tbm(resource_manager::fetch_tbm(imstring("assets/intro.tbm")));
  m_splash.set_tbm(logo_tbm);
  m_splash.set_frame(0, 0, logo_tbm.width(), logo_tbm.height(), 1);
  tbm cover_tbm(
      resource_manager::fetch_tbm(imstring("6gallery/viewer/CT-IMBIBE.tbm")));
  m_cover.set_tbm(cover_tbm);
  m_cover.set_frame(0, 0, cover_tbm.width(), cover_tbm.height(), 2);
  m_active = false;
}

void intro_element::layout(coord_t window_width, coord_t window_height) {
  set_frame(0, 0, window_width, window_height);
  m_capture_background.set_frame(0, 0, window_width, window_height, 0);
  m_capture_background.set_owner(this);
  m_capture_background.show();
  m_splash.set_frame_pos((window_width - m_splash.frame().width()) / 2,
                         (window_height - m_splash.frame().height()) / 2);
  m_splash.set_owner(this);
  m_splash.show();
  m_cover.set_owner(this);
  m_cover.show();
}

void intro_element::poll() {
  if (m_active && m_cover_y.done()) {
    application::do_next_from_intro();
    m_active = false;
  }
}

bool intro_element::handle_key(key_code_t key) {
  (void)key;
  m_logo_fade.finish();
  m_cover_y.finish();
  return false;
}

void intro_element::animate(anim_time_t delta_ms) {
  m_cover_y.update(delta_ms);
  coord_t cover_y = m_cover_y.value();
  m_cover.set_frame_pos(0, cover_y);

  m_capture_background.set_visible(cover_y > 0);
  m_splash.set_visible(cover_y > 0);

  m_logo_fade.update(delta_ms);
  m_splash.set_fade(m_logo_fade.value());
}

bool intro_element::active() const { return m_active; }

bool intro_element::opaque() const {
  return m_cover.frame().y2 >= frame().height();
}

void intro_element::set_capture(bitmap const &n_capture) {
  m_capture_background.set_b(n_capture);
}

void intro_element::play_intro() {
  m_logo_fade.reset(0, termviz::fade_steps - 1, 1000);
  m_cover_y.reset(frame().height(), -m_cover.frame().height(),
                  120 * m_cover.frame().height(), 6500);
  m_active = true;
}
