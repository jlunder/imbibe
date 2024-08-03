#include "imbibe.hh"

#include <iostream.h>
#include <stdlib.h>
#include "intro.hh"
#include "outro.hh"
#include "task.hh"
#include "task_manager.hh"
#include "text_window.hh"


class menu_task: public task
{
public:
  menu_task(task_manager & n_owner, char const * mnu_path): task(n_owner) {}
  bool done() {return true;}
};


class main_task: public task
{
public:
  main_task(task_manager & n_owner);
  virtual void begin();
  virtual bool run();
  virtual void end();

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


main_task::main_task(task_manager & n_owner):
  task(n_owner), m_intro(n_owner, "intro.bin"), m_menu(n_owner, "main.mnu"), m_outro(n_owner, "outro.bin"), m_state(waiting_for_start)
{
}


void main_task::begin()
{
  m_intro.start();
  m_state = waiting_for_intro;
}


bool main_task::run()
{
  switch(m_state)
  {
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
    cout << "BIG FAT ERROR #2493!" << endl;
    abort();
    break;
  }

  return true;
}


void main_task::end()
{
  m_state = waiting_for_start;
}


void run()
{
  text_window w;
  task_manager t;
  main_task m(t);

  m.start();
  t.run();
}


int main(int argc, char * argv[])
{
  cout << "imbibe 1.0 loaded" << endl;
  run();
  cout << "imbibe 1.0 done" << endl;
  cout << "  code courtesy of hacker joe" << endl;
  return 0;
}


