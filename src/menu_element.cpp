#include "imbibe.h"

#include "menu_element.h"

#include "application.h"
#include "keyboard.h"
#include "resource_manager.h"
#include "tbm.h"
#include "termviz.h"

#define logf_menu_element(...) disable_logf("MENU_ELEMENT: " __VA_ARGS__)

namespace aux_menu_element {

static struct {
  char const *name;
} option_defs[] = {{"menu-se0.tbm"}, {"menu-se1.tbm"}, {"menu-se2.tbm"},
                   {"menu-se3.tbm"}, {"menu-se4.tbm"}, {"menu-se5.tbm"},
                   {"menu-se6.tbm"}};

static const segsize_t options_length = LENGTHOF(option_defs);

} // namespace aux_menu_element
// first hotspot lines 11-17

menu_element::menu_element() : m_data_arena(), m_menu_options() {}
//    : m_data_arena(10000, "menu_element"), m_menu_options(&m_data_arena) {}

void menu_element::layout(coord_t window_width, coord_t window_height) {
  (void)window_width;
  (void)window_height;
  coord_t width;
  {
    // with_arena a(&m_data_arena);
    m_background = resource_manager::fetch_tbm("menu-bg.tbm");
    width = m_background.header().width;
    m_menu_options.reserve(aux_menu_element::options_length);
    coord_t hot_y1 = 11;
    coord_t hot_y2 = 17;
    coord_t sel_y = 13;
    for (segsize_t i = 0; i < aux_menu_element::options_length; ++i) {
      m_menu_options.push_back();
      menu_option *o = &m_menu_options.back();
      o->hot.assign(0, hot_y1, width, hot_y2);
      o->selected_pos = point(0, sel_y);
      o->selected_overlay =
          resource_manager::fetch_tbm(aux_menu_element::option_defs[i].name);
      logf_menu_element("loaded TBM %s: %d x %d\n",
                        aux_menu_element::option_defs[i].name,
                        (int)o->selected_overlay.header().width,
                        (int)o->selected_overlay.header().height);
      o->hide_transition.reset(width, width, 0);
      o->show_transition.reset(width, width, 0);
      hot_y1 += 16;
      hot_y2 += 16;
      sel_y += 16;
    }
  }
  set_frame(0, 0, m_background.header().width, m_background.header().height);
  // m_data_arena.trim();
  m_scroll_y.reset(0, 0, 0);
}

void menu_element::poll() {}

bool menu_element::handle_key(uint16_t key) {
  switch (key) {
  case key_code::escape:
    application::do_quit_from_anywhere();
    return true;

  case key_code::up:
  case key_code::pgup:
    m_selected_option = max<segsize_t>(m_selected_option, 1) - 1;
    break;

  case key_code::down:
  case key_code::pgdown:
    m_selected_option = min(m_selected_option + 1, m_menu_options.size() - 1);
    break;

  case key_code::home:
    m_selected_option = 0;
    break;

  case key_code::end:
    m_selected_option = m_menu_options.size() - 1;
    break;

  case key_code::right:
  case key_code::enter:
  case ' ':
    break;
  }
  return false;
}

bool menu_element::active() const { return true; }

void menu_element::animate(anim_time_t delta_ms) {
  if (m_selected_option != m_last_selected_option) {
    request_repaint();
    m_last_selected_option = m_selected_option;
  }
  for (segsize_t i = 0; i < m_menu_options.size(); ++i) {
    m_menu_options[i].hide_transition.update(delta_ms);
    m_menu_options[i].show_transition.update(delta_ms);
  }
  m_scroll_y.update(delta_ms);
}

void menu_element::paint(graphics *g) {
  screen_element::paint(g);
  g->draw_tbm(0, 0, m_background);
  for (segsize_t i = 0; i < m_menu_options.size(); ++i) {
    menu_option *o = &m_menu_options[i];
    if (i == m_selected_option) {
      g->draw_tbm(o->selected_pos.x, o->selected_pos.y, o->selected_overlay);
    }
  }
}
