static_runtime_error::static_runtime_error(char const * n_message) throw():
  m_message(n_message)
{
}


virtual char const * static_runtime_error::message() throw()
{
  return m_message;
}


static_logic_error::static_logic_error(char const * n_message) throw():
  m_message(n_message)
{
}


virtual char const * static_logic_error::message() throw()
{
  return m_message;
}


static_interrupt::static_interrupt(char const * n_source) throw():
  m_source(n_source)
{
}


virtual char const * static_interrupt::source() throw()
{
  return m_source;
}


