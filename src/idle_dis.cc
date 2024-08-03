#include "imbibe.hh"

#include "idle_dispatcher.hh"

#include "idle_handler.hh"
#include "vector.hh"

#include "idle_dispatcher.ii"

#include "idle_handler.ii"
#include "vector.ii"


idle_dispatcher::idle_dispatcher():
  m_busy(false)
{
}


void idle_dispatcher::add_task(idle_handler & ih)
{
  m_idle_handlers.push_back(&ih);
  ih.begin();
}


void idle_dispatcher::remove_task(idle_handler & ih)
{
  idle_handler_p_list::iterator i;

  for(i = m_idle_handlers.begin(); (i != m_idle_handlers.end()) && (*i != &ih); ++i);
  assert(i != m_idle_handlers.end());
  m_idle_handlers.erase(i);
}


void idle_dispatcher::idle()
{
  idle_handler_p_list::iterator i;

  for(i = m_idle_handlers.begin(); i != m_idle_handlers.end(); ++i)
  {
    (*i)->idle();
  }
}


