#ifndef __INPLACE_H_INCLUDED
#define __INPLACE_H_INCLUDED


template<class T>
class inplace {
public:
  void setup() { new (&**this) T; }
  void teardown() { (*this)->~T(); }

  operator T * () { return (T *)m_buf; }
  T & operator * () { return *(T *)m_buf; }
  T * operator -> () { return (T *)m_buf; }

private:
  uint32_t m_buf[sizeof (T) / sizeof (uint32_t)];
};


#endif // __INPLACE_H_INCLUDED

