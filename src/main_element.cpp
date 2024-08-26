#include "imbibe.h"

#include "main_element.h"

#include "immutable.h"
#include "keyboard.h"
#include "main_task.h"
#include "resource_manager.h"
#include "tbm.h"
#include "termviz.h"
#include "tweens.h"


#define logf_main_element(...) logf("MAIN_ELEMENT: " __VA_ARGS__)


main_element::main_element()
  : window_element(), m_state(st_init),
    m_prop_fade(0),
    m_prop_submenu_slide(0), m_prop_cover_scroll(0), m_logo_background(),
    m_logo(), m_cover(), m_menu(), m_menu_header(), m_menu_footer(),
    m_menu_background() {
    // , m_submenu(), m_submenu_header(), m_submenu_footer(),
    // m_submenu_background() {
}


main_element::~main_element() {
}


void main_element::set_captured_screen(im_ptr<bitmap> captured_screen) {
  m_logo_background.set_b(captured_screen);
}


void main_element::layout() {
  coord_t screen_width = frame_width();
  coord_t screen_height = frame_height();

  unpacker const & logo_data = resource_manager::fetch_tbm("logo.tbm");
  coord_t logo_width;
  coord_t logo_height;
  tbm::dimensions(logo_data, logo_width, logo_height);
  coord_t logo_x = (screen_width - logo_width) / 2;
  coord_t logo_y = (screen_height - logo_height) / 2;
  unpacker const & menu_header_data = resource_manager::fetch_tbm("menu-top.tbm");
  coord_t menu_header_width;
  coord_t menu_header_height;
  tbm::dimensions(menu_header_data, menu_header_width, menu_header_height);
  unpacker const & menu_footer_data = resource_manager::fetch_tbm("menu-bot.tbm");
  coord_t menu_footer_width;
  coord_t menu_footer_height;
  tbm::dimensions(menu_footer_data, menu_footer_width, menu_footer_height);

  m_logo_background.set_owner(*this);
  // captured screen received above
  m_logo.set_owner(*this);
  m_cover.set_owner(*this);
  m_cover.set_b(resource_manager::fetch_bitmap("cover.tbm"));
  m_menu.set_owner(*this);
  m_menu_background.set_owner(m_menu);
  m_menu_background.set_brush(termel::from('.', color::black, color::white));
  m_menu_background.show();
  m_menu_header.set_owner(m_menu);
  m_menu_header.set_tbm(menu_header_data);
  m_menu_header.show();
  m_menu_footer.set_owner(m_menu);
  m_menu_footer.set_tbm(menu_footer_data);
  m_menu_footer.show();
  // m_submenu.set_owner(*this);
  // m_submenu_background.set_owner(m_submenu);
  // m_submenu_header.set_owner(m_submenu);
  // m_submenu_footer.set_owner(m_submenu);

  m_logo.set_tbm(logo_data);

  m_menu.set_frame(0, 0, screen_width, screen_height, 0);
  m_menu_background.set_frame(0, 0, screen_width, screen_height, 0);
  m_menu_header.set_frame(0, 0, screen_width,
    screen_height - menu_header_height, 1);
  m_menu_footer.set_frame(0, screen_height - menu_footer_height,
    screen_width, screen_height, 2);

  m_logo_background.set_frame(0, 0, screen_width, screen_height, 1);
  m_logo.set_frame(logo_x, logo_y, logo_x + logo_width, logo_y + logo_height,
    2);
  m_cover.set_frame(0, 0, screen_width, m_cover.b().height(), 3);

  // propagate to children once we have set their dimensions from ours
  window_element::layout();
}


void main_element::animate(uint32_t delta_ms) {
  if (m_state == st_init) {
    enter_intro();
  } else if ((m_state == st_intro) && (m_anim_timer.done())) {
    enter_main_menu();
  }
  // } else if ((m_state == st_intro) && (m_animator.last_time() >= 2250)) {
  //   enter_main_menu();
  // }

  // m_animator.animate(delta_ms);
  assert(delta_ms < (uint32_t)(ANIM_TIME_MAX / 8));
  m_logo_fade.update((anim_time_t)delta_ms);
  m_cover_scroll.update((anim_time_t)delta_ms);
  m_anim_timer.update((anim_time_t)delta_ms);

  coord_t screen_height = frame_height();
  coord_t cover_height = m_cover.frame_height();

  bool logo_visible = m_cover_scroll.value() > 0;
  bool cover_visible = (m_cover_scroll.value() < screen_height)
    && (m_cover_scroll.value() + cover_height > 0);
  bool menu_visible =
    (m_cover_scroll.value() + cover_height) < screen_height;

  lock_repaint();

  m_logo_background.set_visible(logo_visible);
  m_logo.set_visible(logo_visible);
  m_logo.set_fade(m_logo_fade.value());
  m_cover.set_visible(cover_visible);
  m_menu.set_visible(menu_visible);

  m_cover.set_frame_pos(0, m_cover_scroll.value());

  unlock_repaint();

  // bool logo_visible = m_prop_cover_scroll > 0;
  // m_logo_background.set_visible(logo_visible);
  // m_logo.set_visible(logo_visible);

  // bool cover_visible = (m_prop_cover_scroll < screen_height)
  //   && (m_prop_cover_scroll + cover_height > 0);
  // m_cover.set_visible(cover_visible);
  // m_cover.set_frame_pos(0, m_prop_cover_scroll);

  // bool menu_visible =
  //   m_prop_cover_scroll < (screen_height - cover_height);
  // m_menu.set_visible(menu_visible);



  // bool submenu_visible = m_prop_submenu_slide < screen_width;
  // m_submenu.set_visible(submenu_visible);
  // m_submenu.set_frame_pos(m_prop_submenu_slide, 0);

}


bool main_element::handle_key(uint16_t key) {
  logf_main_element("handle_key: %X\n", key);
  if (key == key_code::escape) {
    main_task::exit();
    return true;
  }
  return false;
}


void main_element::enter_intro() {
  m_state = st_intro;
  request_repaint();

  coord_t screen_width = frame_width();
  coord_t screen_height = frame_height();
  coord_t cover_height = m_cover.frame_height();

  m_prop_fade = 0;
  m_prop_submenu_slide = screen_width;
  m_prop_cover_scroll = screen_height;

  // m_animator.clear_all();
  // m_animator.play(k_logo_fade, 0, 250,
  //   tweens::linear<uint8_t>(&m_prop_fade, 0, termviz::fade_steps));
  // m_animator.play(k_cover_scroll, 250, 2000,
  //   tweens::linear<coord_t>(&m_prop_cover_scroll, screen_height, -cover_height));
  m_logo_fade.reset(0, termviz::fade_steps - 1, 1000);
  m_cover_scroll.reset(screen_height, -cover_height, 5000, 2000);
  m_anim_timer.reset(11000);

}


void main_element::enter_main_menu() {
  logf_main_element("entering main_menu state\n");
  m_state = st_main_menu;

  m_prop_fade = 0;
  m_prop_submenu_slide = frame_width();
  m_prop_cover_scroll = -m_cover.frame_height();

  // m_animator.clear_all();
}


void main_element::enter_outro() {
  logf_main_element("entering outro state\n");
  m_state = st_outro;

  m_prop_fade = 0;
  m_prop_submenu_slide = frame_width();
  m_prop_cover_scroll = -m_cover.frame_height();
  // m_animator.clear_all();

  // m_prop_fade = 0;
  // m_prop_submenu_slide = frame_width();
  // m_prop_cover_scroll = -m_cover.frame_height();
  // m_animator.clear_all();
  // m_animator.play(k_submenu_slide, 0, 250,
  //   tweens::ease_out_quad<coord_t>(&m_prop_submenu_slide, m_prop_submenu_slide, 0));

  // m_prop_fade = 0;
  // m_prop_submenu_slide = 0;
  // m_prop_cover_scroll = -m_cover.frame_height();
  // m_animator.clear_all();
  // m_animator.play(k_submenu_slide, 0, 250,
  //   tweens::ease_in_quad<coord_t>(&m_prop_submenu_slide, m_prop_submenu_slide, screen_width));
}


