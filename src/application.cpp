#include "imbibe.h"

#include "application.h"

#include "bitmap_element.h"
#include "immutable.h"
#include "inplace.h"
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

static anim_time_t const s_min_poll_interval_ms = 10;
static anim_time_t const s_max_idle_interval_ms = 200;

coord_t s_display_width;
coord_t s_display_height;

mode_t s_last_mode;
imstring s_last_submenu_config;
bool s_last_showing_viewer;
imstring s_last_viewer_article;
bool s_last_showing_quit_prompt;

mode_t s_mode;
imstring s_submenu_config;
bool s_showing_viewer;
imstring s_viewer_article;
bool s_showing_quit_prompt;

bool s_quitting;

timer s_frame_timer;
timer s_idle_timer;

inplace<text_window> s_win;
inplace<window_element> s_main;

inplace<intro_element> s_intro_screen;
inplace<menu_element> s_menu_screen;
inplace<submenu_element> s_submenu_screen;
inplace<viewer_element> s_viewer_screen;
inplace<outro_element> s_outro_screen;
inplace<quit_prompt_element> s_quit_prompt;

void poll_input();
void animate(anim_time_t anim_ms);
void idle();

screen_element &focused_screen();

void internal_do_cancel_prompts();

void activate_intro();
void deactivate_intro();
void activate_menu();
void deactivate_menu();
void activate_submenu(imstring config);
void deactivate_submenu();
void activate_viewer(imstring article);
void deactivate_viewer();
void activate_quit_prompt();
void deactivate_quit_prompt();
void activate_outro();

void update_transitions();

} // namespace application

void application::setup() {
  logf_application("imbibe 0.1 loading...\n");

  s_last_mode = mode_none;
  s_mode = mode_intro;
  s_last_submenu_config = s_submenu_config = NULL;
  s_last_showing_viewer = s_showing_viewer = false;
  s_last_viewer_article = s_viewer_article = NULL;
  s_last_showing_quit_prompt = s_showing_quit_prompt = false;
  s_quitting = false;

  s_win.setup();
  s_main.setup();
  s_intro_screen.setup();
  s_menu_screen.setup();
  s_submenu_screen.setup();
  s_viewer_screen.setup();
  s_outro_screen.setup();
  s_quit_prompt.setup();

  s_win->setup(true);
  s_display_width = s_win->backbuffer()->width();
  s_display_height = s_win->backbuffer()->height();

  s_main->set_frame(0, 0, s_display_width, s_display_height);
  s_menu_screen->set_frame_depth(1);
  s_submenu_screen->set_frame_depth(2);
  s_viewer_screen->set_frame_depth(3);
  s_intro_screen->set_frame_depth(10);
  s_quit_prompt->set_frame_depth(20);
  s_outro_screen->set_frame_depth(30);

  s_main->set_owner(s_win);
  s_menu_screen->set_owner(s_main);
  s_submenu_screen->set_owner(s_main);
  s_viewer_screen->set_owner(s_main);
  s_intro_screen->set_owner(s_main);
  s_quit_prompt->set_owner(s_main);
  s_outro_screen->set_owner(s_main);

  assert(s_win->capture());
  bitmap const &b = *s_win->capture();
  s_intro_screen->set_capture(bitmap(b.width(), b.height(), b.data()));

  s_menu_screen->layout(s_display_width, s_display_height);
  s_submenu_screen->layout(s_display_width, s_display_height);
  s_viewer_screen->layout(s_display_width, s_display_height);
  s_intro_screen->layout(s_display_width, s_display_height);
  s_quit_prompt->layout(s_display_width, s_display_height);
  s_outro_screen->layout(s_display_width, s_display_height);

  logf_application("imbibe loaded -- enjoy!\n");
}

void application::teardown() {
  logf_application("shutting down\n");

  s_main->hide();

  s_menu_screen->hide();
  s_submenu_screen->hide();
  s_viewer_screen->hide();
  s_intro_screen->hide();
  s_quit_prompt->hide();
  s_outro_screen->hide();

  s_win->teardown();

  s_win.teardown();
  s_main.teardown();
  s_intro_screen.teardown();
  s_menu_screen.teardown();
  s_submenu_screen.teardown();
  s_viewer_screen.teardown();
  s_outro_screen.teardown();
  s_quit_prompt.teardown();

  logf_application("bye!\n");
}

void application::run_loop() {
  // cause reset of s_idle_timer
  idle();
  s_frame_timer.read_ms();

  while (!s_quitting) {
    sim::step_poll();
    poll_input();
    update_transitions();

    // logf_application("main loop: check for starvation\n");
    // idle() if it's been a really long time since we last did
    while ((s_idle_timer.peek_ms() >= s_max_idle_interval_ms) ||
           (s_frame_timer.peek_ms() < s_min_poll_interval_ms)) {
      sim::step_idle();
      idle();
    }

    uint32_t frame_ms = s_frame_timer.read_exact_ms(s_min_poll_interval_ms);
    assert(frame_ms >= (anim_time_t)s_min_poll_interval_ms);
    anim_time_t anim_ms =
        (anim_time_t)min<uint32_t>(frame_ms, s_min_poll_interval_ms * 10);
    sim::step_animate(anim_ms);
    s_win->lock_repaint();
    animate(anim_ms);
    s_main->show();
    s_win->unlock_repaint();
    // s_win->repaint(rect(10, 4, 20, 8));
    // s_win->repaint(rect(10, 20, 20, 24));
    // s_win->repaint(rect(60, 4, 70, 8));
    // s_win->repaint(rect(60, 20, 70, 20));
    s_win->present();
  }
}

void application::poll_input() {
  focused_screen().poll();
  while (keyboard::key_event_available()) {
    uint16_t key = keyboard::read_key_event();
    logf_application("key pressed: %X\n", key);
    if (focused_screen().handle_key(key)) {
      continue;
    }
    switch (key) {
    case 'q':
    case 'x':
      do_quit_from_anywhere();
      continue;
    case 'Q': // shift_q
    case 'X': // shift_x
    case key_code::control_q:
    case key_code::control_x:
    case key_code::alt_q:
    case key_code::alt_x:
      do_immediate_quit_from_anywhere();
      continue;
    default:
      break;
    }
  }
}

void application::animate(anim_time_t anim_ms) {
  screen_element *screens[] = {
      &*s_quit_prompt,   &*s_outro_screen,   &*s_intro_screen,
      &*s_viewer_screen, &*s_submenu_screen, &*s_menu_screen,
  };

  bool found_active = false;
  bool found_opaque = false;
  for (size_t i = 0; i < LENGTHOF(screens); ++i) {
    if (found_opaque || (!found_active && !screens[i]->active())) {
      screens[i]->hide();
    } else {
      if (screens[i]->active()) {
        found_active = true;
        // show() after animate() because coming into this method, animated
        // positions may need to be initialized -- this avoids weird pops and
        // over-rendering
        screens[i]->animate(anim_ms);
        screens[i]->show();
        if (screens[i]->opaque()) {
          found_opaque = true;
        }
      } else {
        screens[i]->hide();
      }
    }
  }
}

void application::idle() { s_idle_timer.read_ms(); }

screen_element &application::focused_screen() {
  if (s_mode == mode_outro) {
    return *s_outro_screen;
  } else if (s_showing_quit_prompt) {
    return *s_quit_prompt;
  } else if (s_showing_viewer) {
    return *s_viewer_screen;
  } else {
    switch (s_mode) {
    case mode_intro:
      return *s_intro_screen;
    case mode_menu:
      return *s_menu_screen;
    case mode_submenu:
      return *s_submenu_screen;
    // special-cased at start of if-else-if above
    // case mode_outro:
    //   return *s_outro_screen;
    default:
      assert(!"invalid s_last_mode");
      return *s_outro_screen;
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
  assert(!s_showing_viewer);
  assert(!s_showing_quit_prompt);
  internal_do_cancel_prompts();

  s_mode = mode_menu;
}

void application::do_submenu_from_menu(imstring config) {
  assert(s_mode == mode_menu);
  assert(!s_showing_viewer);
  assert(!s_showing_quit_prompt);
  internal_do_cancel_prompts();
  s_submenu_screen->enter_from_menu();
  s_submenu_config = config;
  s_mode = mode_submenu;
}

void application::do_viewer_from_menu(imstring article) {
  assert(s_mode == mode_menu);
  assert(!s_showing_viewer);
  assert(!s_showing_quit_prompt);
  s_viewer_article = article;
  s_showing_viewer = true;
}

void application::do_viewer_from_submenu(imstring article) {
  assert(s_mode == mode_submenu);
  assert(!s_showing_viewer);
  assert(!s_showing_quit_prompt);
  s_submenu_screen->leave_to_viewer();
  s_viewer_article = article;
  s_showing_viewer = true;
}

void application::do_back_from_submenu() {
  assert(s_mode == mode_submenu);
  assert(!s_showing_viewer);
  assert(!s_showing_quit_prompt);
  s_submenu_screen->leave_to_menu();
  s_mode = mode_menu;
}

void application::do_back_from_viewer() {
  assert((s_mode == mode_menu) || (s_mode == mode_submenu));
  assert(s_showing_viewer);
  assert(!s_showing_quit_prompt);
  if (s_mode == mode_submenu) {
    s_submenu_screen->enter_from_viewer();
  }
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
    switch (s_last_mode) {
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
    switch (s_mode) {
    case mode_intro:
      activate_intro();
      break;
    case mode_menu:
      activate_menu();
      break;
    case mode_submenu:
      activate_submenu(s_submenu_config);
      break;
    case mode_outro:
      activate_outro();
      break;
    default:
      assert(!"invalid s_mode");
      break;
    }
  }
}

void application::activate_intro() {
  assert(s_last_mode == mode_none);
  s_intro_screen->play_intro();
  s_last_mode = mode_intro;
}

void application::deactivate_intro() {
  assert(s_last_mode == mode_intro);
  s_last_mode = mode_none;
}

void application::activate_menu() {
  assert(s_last_mode == mode_none);
  s_last_mode = mode_menu;
  s_menu_screen->activate();
}

void application::deactivate_menu() {
  assert(s_last_mode == mode_menu);
  s_last_mode = mode_none;
}

void application::activate_submenu(imstring config) {
  assert(s_last_mode == mode_none);
  assert(!config.null_or_empty());
  s_submenu_screen->activate(s_submenu_config);
  s_last_submenu_config = config;
  s_last_mode = mode_submenu;
}

void application::deactivate_submenu() {
  assert(s_last_mode == mode_submenu);
  s_submenu_screen->deactivate();
  s_last_mode = mode_none;
}

void application::activate_viewer(imstring article) {
  assert((s_last_mode == mode_menu) || (s_last_mode == mode_submenu));
  assert(!article.null_or_empty());
  s_last_viewer_article = article;
  s_last_showing_viewer = true;
}

void application::deactivate_viewer() {
  assert(s_last_showing_viewer);
  s_last_viewer_article = NULL;
  s_last_showing_viewer = false;
}

void application::activate_quit_prompt() {
  assert(!s_last_showing_quit_prompt);
  s_quit_prompt->prompt_quit();
  s_last_showing_quit_prompt = true;
}

void application::deactivate_quit_prompt() {
  assert(s_last_showing_quit_prompt);
  s_last_showing_quit_prompt = false;
}

void application::activate_outro() {
  assert(s_last_mode == mode_none);
  s_last_mode = mode_outro;
}
