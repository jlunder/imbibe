#include "imbibe.h"

#include "submenu_element.h"

#include "application.h"
#include "iff.h"
#include "immutable.h"
#include "keyboard.h"
#include "resource_manager.h"
#include "tbm.h"
#include "termviz.h"
#include "unpacker.h"

#define logf_submenu_element(...) logf("SUBMENU_ELEMENT: " __VA_ARGS__)

void submenu_element::layout(coord_t window_width, coord_t window_height) {
  set_frame(0, 0, window_width, window_height);

  if (!try_unpack_menu_config()) {
    assert(!"nnoooooooo");
  }
  // {
  //   width = m_background.width();
  //   m_options.reserve(aux_menu_element::options_length);
  //   coord_t hot_y1 = 10;
  //   coord_t hot_y2 = 16;
  //   coord_t sel_y = 12;
  //   for (segsize_t i = 0; i < aux_menu_element::options_length; ++i) {
  //     m_options.push_back();
  //     menu_option *o = &m_options.back();
  //     o->hot.assign(0, hot_y1, width, hot_y2);
  //     o->selected_pos = point(0, sel_y);
  //     o->selected_overlay =
  //         resource_manager::fetch_tbm(aux_menu_element::option_defs[i].name);
  //     logf_menu_element("loaded TBM %s: %d x %d\n",
  //                       aux_menu_element::option_defs[i].name,
  //                       (int)o->selected_overlay.width(),
  //                       (int)o->selected_overlay.height());
  //     o->hide_transition.reset(0, 0, 0);
  //     o->show_transition.reset(0, 0, 0);
  //     hot_y1 += 16;
  //     hot_y2 += 16;
  //     sel_y += 16;
  //   }
  // }

  m_scroll_y.reset(0, 0, 0);

  m_selected_option = 0;
  m_last_selected_option = SEGSIZE_INVALID;
}

bool submenu_element::try_unpack_menu_config() {
  immutable cfg_im;
  segsize_t cfg_size =
      resource_manager::fetch_data(application::s_menu_cfg_path, &cfg_im);
  if (!cfg_im) {
    return false;
  }

  unpacker data(cfg_im.data(), cfg_size);
  uint32_t data_size;
  if (!iff::try_expect_magic(&data, FOURCC("CFmn"), &data_size) ||
      !data.try_subrange((segsize_t)data_size)) {
    return false;
  }

  segsize_t submenus_count;
  if (!data.try_skip_string() || !data.try_unpack(&submenus_count)) {
    return false;
  }

  for (segsize_t i = 0; i < submenus_count; ++i) {
    imstring submenu_path;
    if (!data.try_skip<coord_t>() || !data.try_skip<coord_t>() ||
        !data.try_skip<coord_t>() || !data.try_skip<coord_t>() ||
        !data.try_skip<coord_t>() || !data.try_skip<coord_t>() ||
        !data.try_skip_string() || !data.try_unpack_string(&submenu_path)) {
      return false;
    }
    m_submenus.push_back();
    if (!try_unpack_submenu_config(submenu_path, &m_submenus.back())) {
      return false;
    }
    m_submenus_by_name.insert(
        map<imstring, submenu *>::value_type(submenu_path, &m_submenus.back()));
  }

  return true;
}

bool submenu_element::try_unpack_submenu_config(imstring const &cfg_path,
                                                submenu *out_submenu) {
  coord_t const window_height = frame().height();

  immutable cfg_im;
  segsize_t cfg_size = resource_manager::fetch_data(cfg_path, &cfg_im);
  if (!cfg_im) {
    return false;
  }

  unpacker data(cfg_im.data(), cfg_size);
  uint32_t data_size;
  if (!iff::try_expect_magic(&data, FOURCC("CFsm"), &data_size) ||
      !data.try_subrange((segsize_t)data_size)) {
    return false;
  }

  // unpack header struct
  imstring menu_header_name;
  imstring menu_footer_name;
  imstring option_unselected_background_name;
  imstring option_selected_background_name;
  segsize_t submenu_option_count;
  if (!data.try_unpack_string(&menu_header_name) ||
      !data.try_unpack_string(&menu_footer_name) ||
      !data.try_unpack_string(&option_unselected_background_name) ||
      !data.try_unpack_string(&option_selected_background_name) ||
      !data.try_unpack(&out_submenu->option_selected_offset) ||
      !data.try_unpack(&out_submenu->option_label_offset) ||
      !data.try_unpack(&out_submenu->option_unselected_label_attribute) ||
      !data.try_unpack(&out_submenu->option_selected_label_attribute) ||
      !data.try_unpack(&submenu_option_count)) {
    return false;
  }
  if (submenu_option_count < 1) {
    return false;
  }

  // load resources referenced in header
  out_submenu->menu_header = resource_manager::fetch_tbm(menu_header_name);
  out_submenu->menu_footer = resource_manager::fetch_tbm(menu_footer_name);
  out_submenu->option_unselected_background =
      resource_manager::fetch_tbm(option_unselected_background_name);
  out_submenu->option_selected_background =
      resource_manager::fetch_tbm(option_selected_background_name);
  // We are conservative about rejecting too-tall headers, footers, and
  // backgrounds, but for sure the header + background height and footer +
  // background height must be <= the window_height or paging calculations will
  // blow up later and real weird things will probably happen
  if (!out_submenu->menu_header.valid() || !out_submenu->menu_footer.valid() ||
      !out_submenu->option_unselected_background.valid() ||
      !out_submenu->option_selected_background.valid() ||
      (out_submenu->menu_header.height() +
           out_submenu->option_unselected_background.height() * 2 >
       window_height) ||
      (out_submenu->menu_footer.height() +
           out_submenu->option_unselected_background.height() * 2 >
       window_height) ||
      (out_submenu->option_unselected_background.height() >
       window_height / 4)) {
    return false;
  }

  // compute layout
  if ((5000 / submenu_option_count) == 0) {
    logf_submenu_element(
        "Too many options (%u) in submenu " PRpF
        " (or background is too tall, %dtm)\n",
        (unsigned)submenu_option_count, cfg_path.c_str(),
        (int)out_submenu->option_unselected_background.height());
  }
  coord_t all_options_y = (coord_t)out_submenu->menu_header.height();
  coord_t all_options_height =
      (coord_t)(submenu_option_count *
                out_submenu->option_unselected_background.height());
  out_submenu->menu_header_pos = point(0, 0);
  out_submenu->menu_footer_pos = point(0, all_options_y + all_options_height);
  out_submenu->option_origin = point(0, all_options_y);
  out_submenu->option_height =
      (coord_t)out_submenu->option_unselected_background.height();
  out_submenu->menu_height = out_submenu->menu_footer_pos.y +
                             (coord_t)out_submenu->menu_footer.height();
  assert((window_height * 3) / (out_submenu->option_height * 4) > 0);
  out_submenu->page_jump = min<segsize_t>(
      1, (window_height * 3) / (out_submenu->option_height * 4) - 1);
  if (out_submenu->page_jump >= submenu_option_count) {
    out_submenu->page_jump = submenu_option_count - 1;
  }

  out_submenu->options.reserve(submenu_option_count);
  for (segsize_t i = 0; i < submenu_option_count; ++i) {
    out_submenu->options.push_back();
    if (!data.try_unpack_string(&out_submenu->options[i].label) ||
        !data.try_unpack_string(&out_submenu->options[i].view_path)) {
      return false;
    }
  }

  return true;
}

void submenu_element::poll() {}

bool submenu_element::handle_key(key_code_t key) {
  switch (key) {
  case key_code::escape:
    application::do_quit_from_anywhere();
    return true;

  case key_code::up:
    m_selected_option = max<segsize_t>(m_selected_option, 1) - 1;
    break;
  case key_code::pgup:
    m_selected_option =
        max<segsize_t>(m_selected_option, m_submenu->page_jump) -
        m_submenu->page_jump;
    break;

  case key_code::down:
    assert(m_submenu->options.size() > 0);
    m_selected_option =
        min(m_selected_option + 1, m_submenu->options.size() - 1);
    break;
  case key_code::pgdown:
    assert(m_submenu->options.size() > 0);
    m_selected_option = min(m_selected_option + m_submenu->page_jump,
                            m_submenu->options.size() - 1);
    break;

  case key_code::home:
    m_selected_option = 0;
    break;

  case key_code::end:
    m_selected_option = m_submenu->options.size() - 1;
    break;

  case key_code::right:
  case key_code::enter:
  case ' ':
    break;
  }
  return false;
}

bool submenu_element::active() const {
  return m_active || !m_transition_in_out.done();
}

void submenu_element::animate(anim_time_t delta_ms) {
  if (m_selected_option != m_last_selected_option) {
    m_hide_option_transition.reset(0, frame().width(), 300, 150);
    m_show_option_transition.reset(0, frame().width(), 300, 150);
    m_unselected_option = m_last_selected_option;
    m_last_selected_option = m_selected_option;

    coord_t cur_y = m_scroll_y.value();
    coord_t target_y;
    if (m_selected_option == 0) {
      target_y = 0;
    } else if (m_selected_option == m_submenu->options.size() - 1) {
      target_y = m_submenu->menu_height - frame().height();
    } else {
      coord_t header_height = m_submenu->menu_header.height();
      coord_t footer_height = m_submenu->menu_footer.height();
      coord_t option_y = m_submenu->option_origin.y +
                         m_submenu->option_height * (coord_t)m_selected_option;
      coord_t window_lower = option_y + m_submenu->option_height +
                             footer_height - frame().height();
      coord_t window_upper = option_y - header_height;
      assert(window_lower < window_upper);
      target_y =
          clamp<coord_t>(clamp<coord_t>(cur_y, window_lower, window_upper), 0,
                         m_submenu->menu_height - frame().height());
    }
    m_scroll_y.reset(cur_y, target_y,
                     abs(m_scroll_y.value() - target_y) * 1000 / 100);
  }
  m_hide_option_transition.update(delta_ms);
  m_show_option_transition.update(delta_ms);
  m_scroll_y.update(delta_ms);

  set_frame_pos(m_transition_in_out.value(), 0);
  request_repaint();
}

void submenu_element::paint(graphics *g) {
  graphics::subregion_state ss1;
  g->enter_subregion(point(0, -m_scroll_y.value()), frame(), &ss1);

  g->draw_tbm(m_submenu->menu_header_pos.x, m_submenu->menu_header_pos.y,
              m_submenu->menu_footer);
  g->draw_tbm(m_submenu->menu_footer_pos.x, m_submenu->menu_footer_pos.y,
              m_submenu->menu_footer);

  for (segsize_t i = 0; i < m_submenu->options.size(); ++i) {
    if (i == m_selected_option) {
      continue;
    }
    submenu_option const &option = m_submenu->options[i];
    point pos = option.unselected_pos;
    g->draw_tbm(pos.x, pos.y, m_submenu->option_unselected_background);
    pos = option.label_pos;
    g->draw_text(pos.x, pos.y, m_submenu->option_unselected_label_attribute,
                 option.label);
  }

  if (m_unselected_option < m_submenu->options.size()) {
    graphics::subregion_state ss2;
    g->enter_subregion(point(0, 0),
                       rect(m_hide_option_transition.value(), 0,
                            frame().width(), m_submenu->menu_height),
                       &ss2);
    submenu_option const &option = m_submenu->options[m_unselected_option];
    point pos = option.unselected_pos;
    g->draw_tbm(pos.x, pos.y, m_submenu->option_unselected_background);
    pos = option.selected_pos;
    g->draw_tbm(pos.x, pos.y, m_submenu->option_selected_background);
    pos = option.label_pos;
    g->draw_text(pos.x, pos.y, m_submenu->option_selected_label_attribute,
                 option.label);
    g->leave_subregion(&ss1);
  }

  if (m_selected_option < m_submenu->options.size()) {
    graphics::subregion_state ss2;
    g->enter_subregion(
        point(0, 0),
        rect(0, 0, m_hide_option_transition.value(), m_submenu->menu_height),
        &ss2);
    submenu_option const &option = m_submenu->options[m_selected_option];
    point pos = option.unselected_pos;
    g->draw_tbm(pos.x, pos.y, m_submenu->option_unselected_background);
    pos = option.selected_pos;
    g->draw_tbm(pos.x, pos.y, m_submenu->option_selected_background);
    pos = option.label_pos;
    g->draw_text(pos.x, pos.y, m_submenu->option_selected_label_attribute,
                 option.label);
    g->leave_subregion(&ss1);
  }

  g->leave_subregion(&ss1);
}

void submenu_element::activate(imstring const &cfg_path) {
  map<imstring, submenu *>::iterator i = m_submenus_by_name.find(cfg_path);
  if (i == m_submenus_by_name.end()) {
    assert(!"invalid submenu path");
    i = m_submenus_by_name.begin();
  }
  m_submenu = i->ref;
  assert(m_submenu);
  m_last_selected_option = m_submenu->options.size();

  if (m_transition_in_out.done()) {
    assert(!"should be running a transition animation by now");
    m_transition_in_out.reset(0, 0, 0);
  }
  m_active = true;
}

void submenu_element::deactivate() { m_active = false; }

void submenu_element::enter_from_menu() {
  m_transition_in_out.reset(frame().width(), 0, 300);
}

void submenu_element::leave_to_menu() {
  m_transition_in_out.reset(0, frame().width(), 300);
}

void submenu_element::enter_from_viewer() {}

void submenu_element::leave_to_viewer() {}
