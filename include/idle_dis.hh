#ifndef __IDLE_DISPATCHER_HH_INCLUDED
#define __IDLE_DISPATCHER_HH_INCLUDED


class idle_dispatcher;


#include "idle_handler.hh"
#include "vector.hh"


class idle_dispatcher
{
public:
  idle_dispatcher();
  void add_idle_handler(idle_handler & ih);
  void remove_idle_handler(idle_handler & ih);
  void idle();

private:
  typedef vector < idle_handler * > idle_handler_p_list;

  idle_handler_p_list m_idle_handlers;
};


#endif //__IDLE_DISPATCHER_HH_INCLUDED


