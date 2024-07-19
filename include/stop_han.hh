#ifndef __STOP_HANDLER_HH_INCLUDED
#define __STOP_HANDLER_HH_INCLUDED


#include "key_handler.hh"
#include "task.hh"


class stop_handler: public key_handler
{
public:
  stop_handler(task & n_t);
  virtual bool handle(int c);

private:
  task & m_t;
};


#endif //__STOP_HANDLER_HH_INCLUDED


