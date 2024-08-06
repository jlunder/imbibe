#include "imbibe.h"

#include <stdlib.h>

#include "inplace.h"
// #include "key_manager.h"
#include "key_mana.h"
// #include "main_task.h"
#include "main_tas.h"
#include "task.h"
// #include "task_manager.h"
#include "task_man.h"
// #include "text_window.h"
#include "text_win.h"
#include "timer.h"


// #undef logf
// #define logf cprintf


inplace<text_window> text_window_instance;
inplace<main_task> main_instance;


int main(int argc, char * argv[]) {
  main_instance.setup();
  main_instance->start();
  main_instance->run_loop();
  main_instance.teardown();
  return 0;
}


