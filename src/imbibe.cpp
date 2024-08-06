#include "imbibe.h"

#include "inplace.h"
// #include "main_task.h"
#include "main_tas.h"


#undef logf
#define logf cprintf


inplace<main_task> main_instance;


int main(int argc, char * argv[]) {
  (void)argc;
  (void)argv;
  main_instance.setup();
  main_instance->start();
  main_instance->run_loop();
  main_instance.teardown();
  return 0;
}


