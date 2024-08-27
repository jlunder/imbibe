#ifndef __APPLICATION_H_INCLUDED
#define __APPLICATION_H_INCLUDED


#include "imbibe.h"

#include "imstring.h"


namespace application {
  void setup();
  void teardown();

  void run_loop();

  void do_quit_from_anywhere();
  void do_immediate_quit_from_anywhere();
  void do_next_from_intro();
  void do_submenu_from_menu(imstring category);
  void do_viewer_from_menu(imstring article);
  void do_viewer_from_submenu(imstring article);
  void do_back_from_submenu();
  void do_back_from_viewer();
  void do_confirm_from_quit_prompt();
  void do_back_from_quit_prompt();
  void do_next_from_outro();
}


#endif // __APPLICATION_H_INCLUDED


