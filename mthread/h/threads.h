#include "exception.hh"


class mutex
{
public:
  mutex();
  ~mutex();
  void lock();
  bool trylock();
  void unlock();
  bool is_locked();
};


class thread_interrupt: public interrupt
{
};


class cond
{
public:
  enum timed_wait_result {signalled, timed_out};

  cond(mutex & n_mut) throw(error);
  ~cond() throw(error);
  void signal() throw(error);
  void broadcast() throw(error);
  void wait() throw(error, interrupt);
  timed_wait_result timed_wait(time t) throw(error, interrupt);
};


class thread
{
public:
  thread();
  ~thread();
  void execute(bool n_attached = true);
  void suspend() throw(error);
  void join() throw(error, interrupt);
  void detach() throw(error);
  void cancel() throw(error);
  void poll() throw(error, interrupt);
  bool is_attached() throw(error);
  virtual void main() throw(error, interrupt) = 0;
  static thread * current() throw();
private:
  bool m_attached;
};


class system
{
public:
  static void panic_stop() throw();
};


