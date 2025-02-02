#ifndef __APPLICATION_H_INCLUDED
#define __APPLICATION_H_INCLUDED

#include "imbibe.h"

#include "imstring.h"

namespace application {

static char const * const s_intro_config = "intro.cfg";
static char const * const s_menu_config = "menu.cfg";
static char const * const s_outro_config = "outro.cfg";

void setup();
void teardown();

void run_loop();

void do_external_abort();
void do_next_from_intro();
void do_quit_from_anywhere();
void do_confirm_from_quit_prompt();
void do_back_from_quit_prompt();
void do_immediate_quit_from_anywhere();
void do_next_from_outro();
void do_submenu_from_menu(imstring config);
void do_back_from_submenu();
void do_viewer_from_menu_or_submenu(imstring resource);
void do_back_from_viewer();

} // namespace application

#endif // __APPLICATION_H_INCLUDED
