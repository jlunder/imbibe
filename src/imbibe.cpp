#include "imbibe.h"

#include <conio.h>
#include <stdlib.h>

#include "intro.h"
#include "outro.h"
#include "task.h"
// #include "task_manager.h"
#include "task_man.h"
// #include "text_window.h"
#include "text_win.h"


class menu_task: public task
{
public:
  menu_task(char const * mnu_path) {}
  bool done() {return true;}
};


class main_task: public task
{
public:
  main_task();
  bool done() {return true;}
  virtual void run();

private:
  enum state_enum
  {
    waiting_for_start,
    waiting_for_intro,
    waiting_for_menu,
    waiting_for_outro,
    waiting_for_end
  };

  intro_task m_intro;
  menu_task m_menu;
  outro_task m_outro;
  state_enum m_state;
};


main_task::main_task():
  task(), m_intro("intro.bin"), m_menu("main.mnu"), m_outro("outro.bin"), m_state(waiting_for_start)
{
}


void main_task::run()
{
  switch(m_state)
  {
  waiting_for_start:
    m_intro.start();
    m_state = waiting_for_intro;
    break;
  waiting_for_intro:
    if(m_intro.done())
    {
      m_menu.start();
      m_state = waiting_for_menu;
    }
    break;
  waiting_for_menu:
    if(m_menu.done())
    {
      m_outro.start();
      m_state = waiting_for_outro;
    }
    break;
  waiting_for_outro:
    if(m_outro.done())
    {
      stop();
      m_state = waiting_for_end;
    }
    break;
  default:
    cprintf("BIG FAT ERROR #2493!\n");
    abort();
    break;
  }
}


void run()
{
  text_window w;
  main_task m;

  m.start(true);
  while(!m.done()) {
    task_manager::run();
  }
}


int main(int argc, char * argv[])
{
  cprintf("imbibe 1.0 loaded\n");
  //run();
  cprintf("imbibe 1.0 done\n");
  return 0;
}


