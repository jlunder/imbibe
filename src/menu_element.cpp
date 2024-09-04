#include "imbibe.h"

#include "menu_element.h"

#include "application.h"
#include "keyboard.h"
#include "resource_manager.h"
#include "tbm.h"
#include "termviz.h"

namespace aux_menu_element {

static struct {
  char const *name;
} option_defs[] = {{"menu-sel0.tbm"}, {"menu-sel1.tbm"}, {"menu-sel2.tbm"},
                   {"menu-sel3.tbm"}, {"menu-sel4.tbm"}, {"menu-sel5.tbm"},
                   {"menu-sel6.tbm"}};

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
  }
  return false;
}

bool menu_element::active() const { return true; }

void menu_element::animate(anim_time_t delta_ms) {
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
