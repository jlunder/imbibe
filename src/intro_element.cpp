#include "imbibe.h"

#include "intro_element.h"

#include "application.h"
#include "keyboard.h"
#include "resource_manager.h"
#include "tbm.h"

intro_element::intro_element() {
  tbm logo_tbm(resource_manager::fetch_tbm(imstring("assets/logo1.tbm")));
  tbm_header const __far &logo_h = logo_tbm.header();
  m_logo_width = logo_h.width;
  m_logo_height = logo_h.height;
  m_logo.set_tbm(logo_tbm);
  tbm cover_tbm(resource_manager::fetch_tbm(imstring("assets/CT-IMBIBE.tbm")));
  tbm_header const __far &cover_h = cover_tbm.header();
  m_cover_width = cover_h.width;
  m_cover_height = cover_h.height;
  m_cover.set_tbm(cover_tbm);
  m_active = false;
}

void intro_element::layout(coord_t window_width, coord_t window_height) {
  set_frame(0, 0, window_width, window_height);
  m_capture_background.set_frame(0, 0, window_width, window_height, 0);
  m_capture_background.set_owner(this);
  m_capture_background.show();
  coord_t logo_x = (window_width - m_logo_width) / 2;
  coord_t logo_y = (window_height - m_logo_height) / 2;
  m_logo.set_frame(logo_x, logo_y, logo_x + m_logo_width,
                   logo_y + m_logo_height, 1);
  m_cover.set_frame(0, window_height, m_cover_width,
                    window_height + m_cover_height, 2);
  m_logo.set_owner(this);
  m_logo.show();
  m_cover.set_owner(this);
  m_cover.show();
}

void intro_element::poll() {
  if (m_active && (m_cover_y.value() + m_cover_height <= 0)) {
    application::do_next_from_intro();
    m_active = false;
  }
}

bool intro_element::handle_key(key_code_t key) {
  (void)key;
  skip_transition();
  return false;
}

void intro_element::animate(anim_time_t delta_ms) {
  m_cover_y.update(delta_ms);
  coord_t cover_y = m_cover_y.value();
  m_cover.set_frame_pos(0, cover_y);

  m_capture_background.set_visible(cover_y > 0);
  m_logo.set_visible(cover_y > 0);

  m_logo_fade.update(delta_ms);
  m_logo.set_fade(m_logo_fade.value());
}

bool intro_element::active() const { return m_active; }

bool intro_element::opaque() const {
  return m_cover.frame().y2 >= frame().height();
}

void intro_element::set_capture(bitmap const &n_capture) {
  m_capture_background.set_b(n_capture);
}

void intro_element::play_intro() {
  m_logo_fade.reset(0, termviz::fade_steps - 1, 500);
  m_cover_y.reset(frame().height(), -m_cover.frame().height(), 8000, 1000);
  m_active = true;
}

void intro_element::skip_transition() {
  m_logo_fade.finish();
  m_cover_y.finish();
}
