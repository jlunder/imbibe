class error
{
public:
  virtual char const * message() throw() = 0;
};


class runtime_error: public error
{
};


class static_runtime_error: public runtime_error
{
public:
  static_runtime_error(char const * n_message) throw();
  virtual char const * message() throw();
private:
  char const * m_message;
};


class logic_error: public error
{
};


class static_logic_error: public logic_error
{
public:
  static_logic_error(char const * n_message) throw();
  virtual char const * message() throw();
private:
  char const * m_message;
};


class interrupt
{
public:
  virtual char const * source() throw() = 0;
};


class static_interrupt: public interrupt
{
public:
  static_interrupt(char const * n_source) throw();
  virtual char const * source() throw();
private:
  char const * m_source;
};


