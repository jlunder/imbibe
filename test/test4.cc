#include "cplusplus.hh"

#include "cstream.hh"
#include "hbin.hh"
#include "hbin_element.hh"
#include "hbin_menu_handler.hh"
#include "key_dispatcher_task.hh"
#include "rectangle_element.hh"
#include "stop_handler.hh"
#include "task_manager.hh"
#include "text_window.hh"

#include "cstream.ii"
#include "hbin.ii"
#include "hbin_element.ii"
#include "hbin_menu_handler.ii"
#include "key_dispatcher_task.ii"
#include "rectangle_element.ii"
#include "stop_handler.ii"
#include "task_manager.ii"
#include "text_window.ii"


void run()
{
  task_manager tm;
  key_dispatcher_task kdt(tm);
  text_window tw;
  stop_handler sh(kdt);
  rectangle_element r(0, 0, 80, 25, 0, tw, pixel('#', color(color::hi_red, color::red)));
  hbin hb(icstream("about.hbi"));
  hbin_element hbe(3, 1, 3 + 60, 1 + 23, 10, tw, hb);
  hbin_menu_handler mh(hb, hbe);

  tw.add_element(r);
  tw.add_element(hbe);
  kdt.add_handler(sh);
  kdt.add_handler(mh);
  kdt.start();
  tm.run();
}


int main(int argc, char * argv[])
{
  cout << "imbibe 1.0 loaded" << endl;
  run();
  cout << "imbibe 1.0 done" << endl;
  cout << "  code courtesy of hacker joe" << endl;
  return 0;
}


