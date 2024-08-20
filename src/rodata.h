#ifndef __RODATA_H_INCLUDED
#define __RODATA_H_INCLUDED


#include "imbibe.h"


// Generic read-only data block
class rodata {
public:
  static uint16_t const max_reclaimable = UINT8_MAX;

  enum prealloc_t { prealloc };
  typedef void (*reclaim_func_t)(void __far * p);

  rodata(prealloc_t policy, void const * p) {
    (void)policy;
    p = normalize(p);
    m_seg = FP_SEG(p);
    m_index = 0;
    m_offset = (uint8_t)FP_OFF(p);
  }

  rodata(reclaim_func_t f, void const * p) {
    assert(p == normalize(p));
    p = normalize(p);
    m_seg = FP_SEG(p);
    m_offset = (uint8_t)FP_OFF(p);
    init(f);
  }

  rodata(const rodata & other): m_seg(other.m_seg), m_index(other.m_index),
      m_offset(other.m_offset) {
    if (m_index != 0) {
      ref();
    }
  }

  ~rodata() {
    if (m_index != 0) {
      unref();
    }
  }

  void __far * data() const { return MK_FP(m_seg, (void *)(uintptr_t)m_offset); }

private:
  __segment m_seg;
  uint8_t m_index;
  uint8_t m_offset;

  void init(reclaim_func_t f);
  void ref();
  void unref();

  static void const * normalize(void const * p) {
    if((uint16_t)FP_OFF(p) <= UINT8_MAX) {
      return p;
    }
    return MK_FP(FP_SEG(p) + (FP_OFF(p) >> 4), FP_OFF(p) & 0xF);
  }
};


template<class T>
class ro: private rodata {
public:
  explicit ro(T const * n_p) : rodata(reclaim, n_p) { }

  T const * get() const { return (T const *)data(); }
  T const & operator * () const { return *(T const *)data(); }
  T const * operator -> () const { return (T const *)data(); }

private:
  static void reclaim(void * p) {
    delete (T const *)p;
  }
};


#endif // __RODATA_H_INCLUDED

