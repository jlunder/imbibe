#include "imbibe.h"

#include "viewer_element.h"

#include "application.h"
#include "keyboard.h"
#include "resource_manager.h"

void viewer_element::layout(coord_t window_width, coord_t window_height) {
  set_frame(0, 0, window_width, window_height);
  m_background = termel::from(' ', color::white, color::black);

  m_page_jump = frame().height() - 2;
  m_scroll_height = 0;
  m_scroll_y_target = 0;
  m_scroll_y_target_last = 0;

  m_scroll_y.reset(m_scroll_y_target);
  m_viewing_offset = point(0, 0);

  m_transition_in_out.reset(frame().width());
}

void viewer_element::poll() {}

bool viewer_element::handle_key(key_code_t key) {
  switch (key) {
  case key_code::escape:
  case key_code::left:
    application::do_back_from_viewer();
    return true;

  case key_code::up:
    m_scroll_y_target = max<coord_t>(m_scroll_y_target - 1, 0);
    break;
  case key_code::pgup:
    m_scroll_y_target = max<coord_t>(m_scroll_y_target - m_page_jump, 0);
    break;

  case key_code::down:
    m_scroll_y_target = min<coord_t>(m_scroll_y_target + 1, m_scroll_height);
    break;
  case key_code::pgdown:
    m_scroll_y_target =
        min<coord_t>(m_scroll_y_target + m_page_jump, m_scroll_height);
    break;

  case key_code::home:
    m_scroll_y_target = 0;
    break;

  case key_code::end:
    m_scroll_y_target = m_scroll_height;
    break;
  }
  return false;
}

bool viewer_element::active() const {
  return m_active || !m_transition_in_out.done();
}

bool viewer_element::opaque() const {
  return m_active && m_transition_in_out.done();
}

void viewer_element::animate(anim_time_t delta_ms) {
  m_transition_in_out.update(delta_ms);
  m_scroll_y.update(delta_ms);

  if (m_scroll_y_target != m_scroll_y_target_last) {
    m_scroll_y.reset(
        m_scroll_y.value(), m_scroll_y_target,
        min(500, abs(m_scroll_y.value() - m_scroll_y_target) * (1000 / 100)));
    m_scroll_y_target_last = m_scroll_y_target;
  }

  if (!active()) {
    m_viewing = tbm();
  }

  request_repaint();
}

void viewer_element::paint(graphics *g) {
  coord_t view_x = m_transition_in_out.value();
  graphics::subregion_state s;
  g->enter_subregion(
      point(view_x, 0),
      rect(view_x, 0, view_x + frame().width(), frame().height()), &s);
  if (m_viewing.valid()) {
    coord_t view_top = m_viewing_offset.y - m_scroll_y.value();
    rect view_rect = rect(m_viewing_offset.x, view_top,
                          m_viewing_offset.x + m_viewing.width(),
                          view_top + m_viewing.height());
    g->draw_tbm(view_rect.x1, view_rect.y1, m_viewing);
    g->draw_rectangle(0, 0, frame().width(), view_rect.y1, m_background);
    g->draw_rectangle(0, view_rect.y1, view_rect.x1, view_rect.y2,
                      m_background);
    g->draw_rectangle(view_rect.x2, view_rect.y1, frame().width(), view_rect.y2,
                      m_background);
    g->draw_rectangle(0, view_rect.y2, frame().width(), frame().height(),
                      m_background);
  } else {
    g->draw_rectangle(frame(), m_background);
  }
  g->leave_subregion(&s);
}

void viewer_element::activate(imstring const &resource) {
  if (m_transition_in_out.done()) {
    assert(!"should be running a transition animation by now");
    m_transition_in_out.reset(0);
  }

  m_viewing = resource_manager::fetch_tbm(resource);

  if (m_viewing.valid()) {
    if (m_viewing.height() < frame().height()) {
      m_scroll_height = 0;
      m_viewing_offset.y = (frame().height() - m_viewing.height()) / 2;
    } else {
      m_scroll_height = m_viewing.height() - frame().height();
      m_viewing_offset.y = 0;
    }
    m_viewing_offset.x = (frame().width() - m_viewing.width()) / 2;
  } else {
    m_scroll_height = 0;
    m_viewing_offset = point(0, 0);
  }
  m_scroll_y_target = 0;

  m_scroll_y.reset(0);

  m_active = true;
}

void viewer_element::deactivate() { m_active = false; }

void viewer_element::enter_from_menu_or_submenu() {
  m_transition_in_out.reset_from_value(frame().width(), 0, 300,
                                       m_transition_in_out.value());
  m_scroll_y.reset(0);
}

void viewer_element::leave_to_menu_or_submenu() {
  m_transition_in_out.reset_from_value(0, frame().width(), 300,
                                       m_transition_in_out.value());
}
