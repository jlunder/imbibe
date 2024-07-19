#include "cplusplus.hh"

#include <iostream.h>

#include "cstream.hh"
#include "key_dispatcher_task.hh"
#include "menu.hh"
#include "menu_element.hh"
#include "menu_handler.hh"
#include "rectangle_element.hh"
#include "stop_handler.hh"
#include "task_manager.hh"
#include "text_window.hh"

#include "cstream.ii"
#include "key_dispatcher_task.ii"
#include "menu.ii"
#include "menu_element.ii"
#include "menu_handler.ii"
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
  menu m(icstream("temp.m"));
  menu_element me(3, 1, 3 + m.width(), 1 + m.height(), 10, tw, m);
  menu_handler mh(m, me);

  tw.add_element(r);
  tw.add_element(me);
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


