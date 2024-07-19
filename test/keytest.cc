#include "cplusplus.hh"

#include <iostream.h>
#include <iomanip.h>

#include "key_dispatcher_task.hh"
#include "key_handler.hh"
#include "task.hh"
#include "task_manager.hh"

#include "key_dispatcher_task.ii"
#include "key_handler.ii"
#include "task.ii"
#include "task_manager.ii"


class list_handler: public key_handler
{
public:
  list_handler(task & n_st);
  virtual bool handle(int c);

private:
  int m_last;
  task & m_st;
};


list_handler::list_handler(task & n_st):
  m_st(n_st)
{
}


bool list_handler::handle(int c)
{
  cout << hex << setw(4) << c << ": \'" << (char)(c & 0xFF) << "\'" << endl;
  if(c == m_last) m_st.stop();
  m_last = c;

  return true;
}


int main(int argc, char * argv[])
{
  task_manager tm;
  key_dispatcher_task kdt(tm);
  list_handler lh(kdt);

  kdt.add_handler(lh);
  kdt.start();
  tm.run();

  return 0;
}


