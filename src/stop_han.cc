#include "imbibe.hh"

#include "stop_handler.hh"

#include "key_handler.hh"
#include "task.hh"

#include "stop_handler.ii"

#include "key_handler.ii"
#include "task.ii"


stop_handler::stop_handler(task & n_t):
  m_t(n_t)
{
}


bool stop_handler::handle(int c)
{
  switch(c)
  {
  case key_escape:
    m_t.stop();
    return true;
    break;
  default:
    break;
  }
  return false;
}


