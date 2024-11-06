#include "imbibe.h"

#include "viewer_element.h"

#include "application.h"
#include "keyboard.h"
#include "resource_manager.h"

void viewer_element::layout(coord_t window_width, coord_t window_height) {
  set_frame(0, 0, window_width, window_height);
  m_background = termel::from(' ', color::white, color::black);
  m_title_background = termel::from(' ', color::white, color::blue);
  m_transition_in_out.reset(frame().width());
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

bool viewer_element::active() const {
  return m_active || !m_transition_in_out.done();
}

bool viewer_element::opaque() const {
  return m_active && m_transition_in_out.done();
}

void viewer_element::animate(anim_time_t delta_ms) {
  m_transition_in_out.update(delta_ms);
  m_scroll_y.update(delta_ms);

  if (!active()) {
    m_viewing = tbm();
  }
}

void viewer_element::paint(graphics *g) {
  coord_t view_x = m_transition_in_out.value();
  graphics::subregion_state s;
  g->enter_subregion(
      point(view_x, 0),
      rect(view_x, 0, view_x + frame().width(), frame().height()), &s);
  if (m_viewing.valid()) {
    coord_t view_y = m_scroll_y.value();
    g->draw_tbm(0, -view_y, m_viewing);
    if (-view_y + m_viewing.height() < frame().height()) {
      g->draw_rectangle(0, -view_y + m_viewing.height(), frame().width(),
                        frame().height() - 1, m_background);
    }
  } else {
    g->draw_rectangle(frame(), m_background);
  }
  g->draw_rectangle(0, frame().height() - 1, frame().width(), frame().height(),
                    m_title_background);
  g->draw_text(1, frame().height() - 1, termel::attribute(m_title_background),
               m_title.c_str());
  g->leave_subregion(&s);
}

void viewer_element::activate(imstring const &title, imstring const &resource) {
  if (m_transition_in_out.done()) {
    assert(!"should be running a transition animation by now");
    m_transition_in_out.reset(0);
  }

  m_title = title;
  m_viewing = resource_manager::fetch_tbm(resource);
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
