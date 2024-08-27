#include "imbibe.h"

#include "application.h"

#include "bitmap_element.h"
#include "immutable.h"
#include "intro_element.h"
#include "keyboard.h"
#include "menu_element.h"
#include "outro_element.h"
#include "quit_prompt_element.h"
#include "submenu_element.h"
#include "text_window.h"
#include "timer.h"
#include "viewer_element.h"


#define logf_application(...) logf("APPLICATION: " __VA_ARGS__)


namespace application {
  enum mode_t {
    mode_none,
    mode_intro,
    mode_menu,
    mode_submenu,
    mode_outro,
  };

  static uint32_t const s_min_poll_interval_ms = 10;
  static uint32_t const s_max_idle_interval_ms = 200;

  coord_t s_display_width;
  coord_t s_display_height;

  mode_t s_last_mode;
  imstring s_last_submenu_category;
  bool s_last_showing_viewer;
  imstring s_last_viewer_article;
  bool s_last_showing_quit_prompt;

  mode_t s_mode;
  imstring s_submenu_category;
  bool s_showing_viewer;
  imstring s_viewer_article;
  bool s_showing_quit_prompt;

  bool s_quitting;

  timer s_frame_timer;
  timer s_idle_timer;

  text_window s_win;
  window_element s_container;

  bitmap s_capture;
  bitmap_element s_capture_background;
  intro_element s_intro_screen;
  menu_element s_menu_screen;
  submenu_element s_submenu_screen;
  viewer_element s_viewer_screen;
  outro_element s_outro_screen;
  quit_prompt_element s_quit_prompt_dialog;

  void poll_input();
  void animate(uint32_t anim_ms);
  void idle();

  element & find_topmost_screen();

  void internal_do_cancel_prompts();

  void activate_intro();
  void deactivate_intro();
  void activate_menu();
  void deactivate_menu();
  void activate_submenu(imstring category);
  void deactivate_submenu();
  void activate_viewer(imstring article);
  void deactivate_viewer();
  void activate_quit_prompt();
  void deactivate_quit_prompt();
  void activate_outro();

  void update_transitions();
}


void application::setup() {
  s_last_mode = mode_none;
  s_mode = mode_intro;
  s_last_submenu_category = s_submenu_category = NULL;
  s_last_showing_viewer = s_showing_viewer = false;
  s_last_viewer_article = s_viewer_article = NULL;
  s_last_showing_quit_prompt = s_showing_quit_prompt = false;
  s_quitting = false;

  s_win.setup(&s_capture);
  s_display_width = s_win.backbuffer().width();
  s_display_height = s_win.backbuffer().height();

  s_win.lock_repaint();

  s_capture_background.set_frame(0, 0, s_display_width, s_display_height, 0);
  s_menu_screen.set_frame(0, 0, s_display_width, s_display_height, 1);
  s_submenu_screen.set_frame(0, 0,  s_display_width, s_display_height, 2);
  s_viewer_screen.set_frame(0, 0,  s_display_width, s_display_height, 3);
  s_intro_screen.set_frame(0, 0,  s_display_width, s_display_height, 10);
  s_quit_prompt_dialog.set_frame(0, 0,  s_display_width, s_display_height, 20);
  s_outro_screen.set_frame(0, 0,  s_display_width, s_display_height, 30);

  s_capture_background.set_b(im_ptr<bitmap>(immutable::prealloc, &s_capture));

  s_container.set_owner(s_win);
  s_menu_screen.set_owner(s_container);
  s_submenu_screen.set_owner(s_container);
  s_viewer_screen.set_owner(s_container);
  s_intro_screen.set_owner(s_container);
  s_quit_prompt_dialog.set_owner(s_container);
  s_outro_screen.set_owner(s_container);

  s_menu_screen.layout();
  s_submenu_screen.layout();
  s_viewer_screen.layout();
  s_intro_screen.layout();
  s_quit_prompt_dialog.layout();
  s_outro_screen.layout();

  update_transitions();
  animate(s_min_poll_interval_ms);

  s_win.unlock_repaint();

  logf_application("imbibe 0.1 loaded\n");
}


void application::teardown() {
  logf_application("shutting down\n");

  s_win.lock_repaint();

  s_menu_screen.hide();
  s_submenu_screen.hide();
  s_viewer_screen.hide();
  s_intro_screen.hide();
  s_quit_prompt_dialog.hide();
  s_outro_screen.hide();

  s_win.unlock_repaint();

  s_win.teardown();

  logf_application("bye!\n");
}


void application::run_loop() {
  idle();
  s_frame_timer.read_ms();

  while (!s_quitting) {
    step_simulator_loop();

    // logf_application("main loop: check for starvation\n");
    // idle() if it's been a really long time since we last did
    while ((s_idle_timer.peek_ms() >= s_max_idle_interval_ms)
        || (s_frame_timer.peek_ms() < s_min_poll_interval_ms)) {
      step_simulator_idle();
      idle();
    }

    step_simulator_poll();

    poll_input();
    update_transitions();

    uint32_t anim_ms = s_frame_timer.read_exact_ms(s_min_poll_interval_ms);
    assert(anim_ms >= s_min_poll_interval_ms);
    animate(anim_ms);
  }
}


void application::poll_input() {
  while (keyboard::key_event_available()) {
    uint16_t key = keyboard::read_key_event();
    logf_application("key pressed: %X\n", key);
    switch(key) {
    case 'q':
    case 'x':
      do_quit_from_anywhere();
      break;
    case 'Q': // shift_q
    case 'X': // shift_x
    case key_code::control_q:
    case key_code::control_x:
    case key_code::alt_q:
    case key_code::alt_x:
      do_immediate_quit_from_anywhere();
      break;
    default:
      find_topmost_screen().handle_key(key);
    }
  }
}


void application::animate(uint32_t anim_ms) {
  element & topmost = find_topmost_screen();
  // show() after animate() because coming into this method, animated
  // positions may need to be initialized -- this avoids weird pops and
  // over-rendering
  topmost.animate(anim_ms);
  topmost.show();

  s_win.present();
}


void application::idle() {
  s_idle_timer.read_ms();
}


element & application::find_topmost_screen() {
  if (s_last_showing_quit_prompt) {
    return s_quit_prompt_dialog;
  } else if(s_last_showing_viewer) {
    return s_viewer_screen;
  } else {
    switch(s_last_mode) {
    case mode_intro:
      return s_intro_screen;
    case mode_menu:
      return s_menu_screen;
    case mode_submenu:
      return s_submenu_screen;
    case mode_outro:
      return s_outro_screen;
    default:
      assert(!"invalid s_last_mode");
      return s_outro_screen;
    }
  }
}


void application::do_quit_from_anywhere() {
  if (s_mode == mode_outro) {
    return;
  } else if (s_mode == mode_intro) {
    do_immediate_quit_from_anywhere();
  } else {
    s_showing_quit_prompt = true;
  }
}


void application::do_immediate_quit_from_anywhere() {
  internal_do_cancel_prompts();
  s_mode = mode_outro;
}


void application::do_next_from_intro() {
  assert(s_mode == mode_intro);
  assert(!s_showing_viewer); assert(!s_showing_quit_prompt);
  internal_do_cancel_prompts();
  s_mode = mode_menu;
}


void application::do_submenu_from_menu(imstring category) {
  assert(s_mode == mode_menu);
  assert(!s_showing_viewer); assert(!s_showing_quit_prompt);
  internal_do_cancel_prompts();
  s_submenu_category = category;
  s_mode = mode_submenu;
}


void application::do_viewer_from_menu(imstring article) {
  assert(s_mode == mode_menu);
  assert(!s_showing_viewer); assert(!s_showing_quit_prompt);
  s_viewer_article = article;
  s_showing_viewer = true;
}


void application::do_viewer_from_submenu(imstring article) {
  assert(s_mode == mode_submenu);
  assert(!s_showing_viewer); assert(!s_showing_quit_prompt);
  s_viewer_article = article;
  s_showing_viewer = true;
}


void application::do_back_from_submenu() {
  assert(s_mode == mode_submenu);
  assert(!s_showing_viewer); assert(!s_showing_quit_prompt);
  s_mode = mode_menu;
}


void application::do_back_from_viewer() {
  assert((s_mode == mode_menu) || (s_mode == mode_submenu));
  assert(s_showing_viewer); assert(!s_showing_quit_prompt);
  s_showing_viewer = false;
}


void application::do_confirm_from_quit_prompt() {
  assert(s_showing_quit_prompt);
  do_immediate_quit_from_anywhere();
}


void application::do_back_from_quit_prompt() {
  assert(s_showing_quit_prompt);
  assert((s_mode == mode_menu) || (s_mode == mode_submenu));
  s_showing_quit_prompt = false;
}


void application::do_next_from_outro() {
  // this one is immediate
  s_quitting = true;
}


void application::internal_do_cancel_prompts() {
  s_showing_quit_prompt = false;
}


void application::update_transitions() {
  if (s_quitting) {
    return;
  }

  if (s_showing_quit_prompt != s_last_showing_quit_prompt) {
    if (s_showing_quit_prompt) {
      activate_quit_prompt();
    } else {
      deactivate_quit_prompt();
    }
  }

  if (s_showing_viewer != s_last_showing_viewer) {
    if (s_showing_viewer && !s_viewer_article.null_or_empty()) {
      activate_viewer(s_viewer_article);
    } else {
      deactivate_viewer();
    }
  }

  if (s_mode != s_last_mode) {
    switch(s_last_mode) {
    case mode_none:
      // should only happen on startup
      assert(s_mode == mode_intro);
      break;
    case mode_intro:
      deactivate_intro();
      break;
    case mode_menu:
      deactivate_menu();
      break;
    case mode_submenu:
      deactivate_submenu();
      break;
    case mode_outro:
      assert(!"should only leave outro mode to quit?");
      break;
    default:
      assert(!"invalid s_last_mode");
      break;
    }
    switch(s_mode) {
    case mode_intro:
      activate_intro();
      break;
    case mode_menu:
      activate_menu();
      break;
    case mode_submenu:
      activate_submenu(s_submenu_category);
      break;
    case mode_outro:
      assert(!"should only leave outro mode to quit?");
      break;
    default:
      assert(!"invalid s_mode");
      break;
    }
  }
}


void application::activate_intro() {
  assert(s_last_mode == mode_none);
  s_intro_screen.show();
  s_last_mode = mode_intro;
}


void application::deactivate_intro() {
  assert(s_last_mode == mode_intro);
  s_intro_screen.hide();
  s_last_mode = mode_none;
}


void application::activate_menu() {
  assert(s_last_mode == mode_none);
  s_menu_screen.show();
  s_last_mode = mode_menu;
}


void application::deactivate_menu() {
  assert(s_last_mode == mode_menu);
  s_menu_screen.hide();
  s_last_mode = mode_none;
}


void application::activate_submenu(imstring category) {
  assert(s_last_mode == mode_none);
  assert(!category.null_or_empty());
  s_submenu_screen.show();
  s_last_submenu_category = category;
  s_last_mode = mode_submenu;
}


void application::deactivate_submenu() {
  assert(s_last_mode == mode_submenu);
  s_submenu_screen.hide();
  s_last_submenu_category = NULL;
  s_last_mode = mode_none;
}


void application::activate_viewer(imstring article) {
  assert((s_last_mode == mode_menu) || (s_last_mode == mode_submenu));
  assert(!article.null_or_empty());
  s_viewer_screen.show();
  s_last_viewer_article = article;
  s_last_showing_viewer = true;
}


void application::deactivate_viewer() {
  assert(s_last_showing_viewer);
  s_viewer_screen.hide();
  s_last_viewer_article = NULL;
  s_last_showing_viewer = false;
}


void application::activate_quit_prompt() {
  assert(!s_last_showing_quit_prompt);
  s_quit_prompt_dialog.show();
  s_last_showing_quit_prompt = true;
}


void application::deactivate_quit_prompt() {
  assert(s_last_showing_quit_prompt);
  s_quit_prompt_dialog.hide();
  s_last_showing_quit_prompt = false;
}


void application::activate_outro() {
  assert(s_last_mode == mode_none);
  s_outro_screen.show();
  s_last_mode = mode_outro;
}


