#ifndef __INPLACE_H_INCLUDED
#define __INPLACE_H_INCLUDED


#if defined(__WATCOMC__)
#pragma warning 549 9
#endif


template<class T>
class inplace {
public:
  void setup() { new ((void *)m_buf) T; }
  void teardown() { (*this)->~T(); }

  operator T * () { return (T *)m_buf; }
  T & operator * () { return *(T *)m_buf; }
  T * operator -> () { return (T *)m_buf; }

private:
  uint32_t m_buf[(sizeof (T) + sizeof (uint32_t) - 1) / sizeof (uint32_t)];
};


#endif // __INPLACE_H_INCLUDED
